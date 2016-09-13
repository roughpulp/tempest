#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <memory>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}

using namespace std;

// ===========================
// Encoder
// ===========================

class Encoder {
public:

    static std::unique_ptr<Encoder> create(
        std::ostream& out,
        AVCodecID codec_id,
        int width, int size,
        int fps
    );

    Encoder(
        std::ostream& out,
        AVCodecContext* ctx,
        AVFrame* frame
    ):
        out_(out),
        ctx_(ctx),
        frame_(frame) {}

    ~Encoder();

    AVFrame* frame() { return frame_; } 

    void next();

    void close();
    
private:
    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;
    
    int send_frame(AVFrame* frame);
    
    std::ostream& out_;
    AVCodecContext* ctx_;
    AVFrame* frame_;
    AVPacket packet_;
};

Encoder::~Encoder() {
    avcodec_close(this->ctx_);
    av_free(this->ctx_);
    av_freep(&(this->frame_->data[0]));
    av_frame_free(&(this->frame_));
}

std::unique_ptr<Encoder> Encoder::create(
        std::ostream& out,
        AVCodecID codec_id,
        int width, int height,
        int fps
    ) {
    avcodec_register_all();
    
    /* find the video encoder */
    AVCodec* codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        throw std::runtime_error("codec not found");
    }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (ctx == nullptr) {
        throw std::runtime_error("avcodec_alloc_context3 failed");
    }

    /* put sample parameters */
    ctx->bit_rate = 400000;
    /* resolution must be a multiple of two */
    ctx->width = width;
    ctx->height = height;
    /* frames per second */
    ctx->time_base = (AVRational){1,fps};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    ctx->gop_size = 10;
    ctx->max_b_frames = 1;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec_id == AV_CODEC_ID_H264) {
        av_opt_set(ctx->priv_data, "preset", "slow", 0);
    }
    
    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        throw std::runtime_error("avcodec_open2 failed");
    }

    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) {
        throw std::runtime_error("av_frame_alloc failed");
    }
    frame->format = ctx->pix_fmt;
    frame->width  = ctx->width;
    frame->height = ctx->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    {
        int nbytes = av_image_alloc(frame->data, frame->linesize, ctx->width, ctx->height, ctx->pix_fmt, 32);
        if (nbytes < 0) {
            throw std::runtime_error("av_image_alloc failed");
        }
        cout << "av_image_alloc() = " << nbytes << endl;
    }
    
    return std::make_unique<Encoder>(out, ctx, frame);
}

void Encoder::next() {
    send_frame(this->frame_);
}

void Encoder::close()
{
    while (send_frame(nullptr)) {
    }
}

int Encoder::send_frame(AVFrame* frame) {
    int got_output;
    const int res = avcodec_encode_video2(this->ctx_, &this->packet_, frame, &got_output);
    if (res != 0) {
        throw std::runtime_error("avcodec_encode_video2 failed: " + std::to_string(res));
    }
    if (got_output) {
        this->out_.write(reinterpret_cast<char*>(this->packet_.data), this->packet_.size);
        av_packet_unref(&this->packet_);
    }
    return got_output;
}

// ===========================
// Decoder
// ===========================

class Decoder {
public:

    static std::unique_ptr<Decoder> create(
        std::istream& in,
        AVCodecID codec_id);

    Decoder(
        std::istream& in,
        AVCodecContext* ctx,
        AVFrame* frame
    ):
        in_(in),
        ctx_(ctx),
        frame_(frame)
    {
        // set end of buffer to 0 (this ensures that no over-reading happens for damaged MPEG streams)
        memset(buffer_ + BUFFER_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    }

    ~Decoder();

    AVFrame* frame() { return frame_; }
    
    bool next();

private:

    enum { BUFFER_SIZE = 4096 };

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;
    
    int decode();
    bool read_input();
    
    std::istream& in_;
    AVCodecContext* ctx_;
    AVFrame* frame_;
    AVPacket packet_;
    uint8_t buffer_[BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
};

Decoder::~Decoder() {
    avcodec_close(this->ctx_);
    av_free(this->ctx_);
    av_freep(&(this->frame_->data[0]));
    av_frame_free(&(this->frame_));
}

std::unique_ptr<Decoder> Decoder::create(
        std::istream& in,
        AVCodecID codec_id) {
    avcodec_register_all();
    
    /* find the video encoder */
    AVCodec* codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        throw std::runtime_error("codec not found");
    }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (ctx == nullptr) {
        throw std::runtime_error("avcodec_alloc_context3 failed");
    }
    
    if (codec->capabilities & AV_CODEC_CAP_TRUNCATED) {
        ctx->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames
    }
    
    /*
    When AVCodecContext.refcounted_frames is set to 1, the frame is
    reference counted and the returned reference belongs to the
    caller. The caller must release the frame using av_frame_unref()
    when the frame is no longer needed. The caller may safely write
    to the frame if av_frame_is_writable() returns 1.
    When AVCodecContext.refcounted_frames is set to 0, the returned
    reference belongs to the decoder and is valid only until the
    next call to this function or until closing or flushing the
    decoder. The caller may not write to it.
    */    
    ctx->refcounted_frames = 0;
    
    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        throw std::runtime_error("avcodec_open2 failed");
    }

    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) {
        throw std::runtime_error("av_frame_alloc failed");
    }
    
    auto decoder = std::make_unique<Decoder>(in, ctx, frame);
    
    av_init_packet(&decoder->packet_);
    decoder->packet_.data = nullptr;
    decoder->packet_.size = 0;
    
    return decoder;
}

bool Decoder::next() {
    for (;;) {
        while (this->packet_.size > 0) {
            if (decode()) {
                return true;
            }
        }
        if (! read_input()) {
            return decode() != 0;
        }
    }
}

int Decoder::decode() {
    int got_frame;
    const int len = avcodec_decode_video2(this->ctx_, this->frame_, &got_frame, &this->packet_);
    if (len < 0) {
        throw std::runtime_error("avcodec_decode_video2: failed: " + std::to_string(len));
    }
    if (this->packet_.data != nullptr) {
        this->packet_.size -= len;
        this->packet_.data += len;
    }
    return got_frame;
}

bool Decoder::read_input() {
    if (this->in_.eof()) {
        this->packet_.data = nullptr;
        this->packet_.size = 0;
        return false;
    } else {
        this->in_.read(reinterpret_cast<char*>(this->buffer_), BUFFER_SIZE);
        this->packet_.data = this->buffer_;
        this->packet_.size = this->in_.gcount();
        return true;
    }
}

//===========================================
// Other
//===========================================

void make_image2(int tt, AVFrame* frame) {
    frame->pts = tt;
    for (int y = 0; y < frame->height; y++) {
        for (int x = 0; x < frame->width; x++) {
            frame->data[0][y * frame->linesize[0] + x] = x + y + tt * 3;
        }
    }
    for (int y = 0; y < frame->height/2; y++) {
        for (int x = 0; x < frame->width/2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128 + y + tt * 2;
            frame->data[2][y * frame->linesize[2] + x] = 64 + x + tt * 5;
        }
    }
}

void video_encode_example2(AVCodecID codec_id) {
    std::ofstream fout("test.mpg1", std::ios::out | std::ios::binary);
    
    auto encoder = Encoder::create(fout, codec_id, 352, 288, 25);
    {
        for (int tt = 0; tt < 10 * 25; ++tt) {
            make_image2(tt, encoder->frame());
            encoder->next();
        }
    }
    encoder->close();    
    fout.close();
}

void video_decode(AVCodecID codec_id) {
    std::ifstream fin("test.mpg1", std::ios::in | std::ios::binary);
    auto decoder = Decoder::create(fin, codec_id);
    {
        while(decoder->next()) {
            cout << "next frame" << endl << flush;
       }
    }
    fin.close();
}

int main()
{
    AVCodecID codec_id = AV_CODEC_ID_MPEG1VIDEO;
    cout << "encode begin" << endl << flush;
    video_encode_example2(codec_id);
    cout << "encode done" << endl << flush;
    cout << "decode begin" << endl << flush;
    video_decode(codec_id);
    cout << "decode end" << endl << flush;
}
