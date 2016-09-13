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
    
    void send_frame(AVFrame* frame);
    bool receive_packets();
    
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
    send_frame(nullptr);
}

void Encoder::send_frame(AVFrame* frame) {
    for(int ii = 0; ii < 1000; ++ii) {
        const int res = avcodec_send_frame(this->ctx_, frame);
        switch (res) {
        case 0:
            return;
        case AVERROR(EAGAIN):
            // input is not accepted right now - the frame must be
            // resent after trying to read output packets
            if (receive_packets()) {
                break;
            } else {
                //EOS
                return;
            }
        case AVERROR_EOF:
            throw std::runtime_error("avcodec_send_frame failed: AVERROR_EOF, the encoder has been flushed, and no new frames can be sent to it");
            
        case AVERROR(EINVAL):
            throw std::runtime_error("avcodec_send_frame failed: AVERROR(EINVAL), codec not opened, refcounted_frames not set, it is a decoder, or requires flush");
            
        case AVERROR(ENOMEM):
            throw std::runtime_error("avcodec_send_frame failed: AVERROR(ENOMEM), failed to add packet to internal queue, or similar other errors: legitimate decoding errors");
            
        default:
            throw std::runtime_error("avcodec_send_frame failed: " + std::to_string(res));
        }
    }
    throw std::runtime_error("video_encode_send_frame too many loops");
}

bool Encoder::receive_packets()
{
    for (int ii = 0; ii < 1000 * 1000; ++ii) {
        av_init_packet(&this->packet_);
        this->packet_.data = nullptr;    // packet data will be allocated by the encoder
        this->packet_.size = 0;
        const int res = avcodec_receive_packet(this->ctx_, &this->packet_);
        switch (res) {
        case 0:
            this->out_.write(reinterpret_cast<char*>(this->packet_.data), this->packet_.size);
            break;
        case AVERROR(EAGAIN):
            // output is not available right now - user must try to send input
            return true;
        case AVERROR_EOF:
            //the encoder has been fully flushed, and there will be no more output packets
            return false;
        case AVERROR(EINVAL):
            throw std::runtime_error("avcodec_receive_packet: AVERROR(EINVAL), codec not opened, or it is an encoder other errors: legitimate decoding errors");        
        default:
            throw std::runtime_error("avcodec_receive_packet failed: " + std::to_string(res));
        }
    }
    throw std::runtime_error("video_encode_receive_packets too many loops");
}

// ===========================
// Decoder
// ===========================

class Decoder {
public:

    static std::unique_ptr<Decoder> create(
        std::istream& in,
        AVCodecID codec_id,
        std::ostream& out);

    Decoder(
        std::istream& in,
        AVCodecContext* ctx,
        AVFrame* frame,
        std::ostream& out
    ):
        in_(in),
        ctx_(ctx),
        frame_(frame),
        out_(out)
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
    
    bool send_packets();
    AVPacket* read_input();
    
    std::istream& in_;
    std::ostream& out_;
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
        AVCodecID codec_id,
        std::ostream& out) {
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

    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        throw std::runtime_error("avcodec_open2 failed");
    }

    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) {
        throw std::runtime_error("av_frame_alloc failed");
    }
    
    auto decoder = std::make_unique<Decoder>(in, ctx, frame, out);
    
    av_init_packet(&decoder->packet_);
    
    return decoder;
}

bool Decoder::next() {
    for (;;) {
        const int res = avcodec_receive_frame(this->ctx_, this->frame_);
        switch (res) {
        case 0:
            // success, a frame was returned
            return true;
        case AVERROR(EAGAIN):
            // output is not available right now - user must try to send new input
            send_packets();
            break;
        case AVERROR_EOF:
            //the decoder has been fully flushed, and there will be no more output frames
            return false;
        case AVERROR(EINVAL):
            throw std::runtime_error("avcodec_receive_frame: AVERROR(EINVAL), codec not opened, or it is an encoder other negative values: legitimate decoding errors");
        default:
            throw std::runtime_error("avcodec_receive_frame failed: " + std::to_string(res));
        }
    }
}

bool Decoder::send_packets() {
    for (;;) {
        AVPacket* packet = read_input();
        const int res = avcodec_send_packet(this->ctx_, packet);
        switch (res) {
        case 0:
            //success, keep feeding with input
            break;
            
        case AVERROR(EAGAIN):
            // input is not accepted right now - the packet must be resent after trying to read output
            return true;

        case AVERROR_EOF:
            //the decoder has been flushed, and no new packets can be sent to it (also returned if more than 1 flush packet is sent)
            return false;

        case AVERROR(EINVAL):
            throw std::runtime_error("avcodec_send_packet failed: AVERROR(EINVAL): codec not opened, it is an encoder, or requires flush");

        case AVERROR(ENOMEM):
            throw std::runtime_error("avcodec_send_packet failed: AVERROR(ENOMEM): failed to add packet to internal queue, or similar other errors: legitimate decoding errors");
            
        default:
            throw std::runtime_error("avcodec_send_packet failed: unknown error: " + std::to_string(res));
        }
    }
}

AVPacket* Decoder::read_input() {
    if (this->in_.eof()) {
        this->packet_.data = nullptr;
        this->packet_.size = 0;
        return nullptr;
    } else {
        this->in_.read(reinterpret_cast<char*>(this->buffer_), BUFFER_SIZE);
        this->packet_.data = this->buffer_;
        this->packet_.size = this->in_.gcount();
        return &this->packet_;
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
        for (int tt = 0; tt < 25 * 10; ++tt) {
            make_image2(tt, encoder->frame());
            encoder->next();
        }
    }
    encoder->close();    
    fout.close();
}

void video_decode(AVCodecID codec_id) {
    std::ofstream fout("test.mpg1.bis", std::ios::out | std::ios::binary);
    std::ifstream fin("test.mpg1", std::ios::in | std::ios::binary);
    auto decoder = Decoder::create(fin, codec_id, fout);
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
