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
#include <libswscale/swscale.h>
}

using namespace std;

//===========================================
// Scaler
//===========================================

class ImageFormat {
public:
    int width_;
    int height_;
    enum AVPixelFormat format_;

    ImageFormat():
        width_(-1),
        height_(-1),
        format_(AV_PIX_FMT_NONE)
    {}
    
    ImageFormat(int ww, int hh, enum AVPixelFormat fmt):
        width_(ww),
        height_(hh),
        format_(fmt)
    {}
    
};

class AvImage {
public:
    uint8_t* data_[4];
    int linesize_[4];
    
    static std::unique_ptr<AvImage> create(const ImageFormat& fmt, int align);

    AvImage(const ImageFormat& fmt): format_(fmt) {
        data_[0] = nullptr;
        data_[1] = nullptr;
        data_[2] = nullptr;
        data_[3] = nullptr;
        linesize_[0] = 0;
        linesize_[1] = 0;
        linesize_[2] = 0;
        linesize_[3] = 0;
    }
    
    ~AvImage();

    const ImageFormat& format() { return format_; }
    
private:
    ImageFormat format_;
};

std::unique_ptr<AvImage> AvImage::create(const ImageFormat& fmt, int align) {
    auto img  = std::make_unique<AvImage>(fmt);
    const int ret = av_image_alloc(img->data_, img->linesize_, fmt.width_, fmt.height_, fmt.format_, align);
    if (ret < 0) {
        throw std::runtime_error("av_image_alloc: failed: " + std::to_string(ret));
    }
    return img;
}

AvImage::~AvImage() {
    av_freep(&data_[0]);
}

class Scaler {
public:

    static std::unique_ptr<Scaler> create(
        const ImageFormat& src,
        const ImageFormat& dst,
        int flags);

    Scaler(struct SwsContext* ctx):
        ctx_(ctx) {}

    void scale(
        const uint8_t* const srcSlice[], const int srcStride[],
        int srcSliceY, int srcSliceH,
        uint8_t* const dstSlice[], const int dstStride[]);
        
    ~Scaler();

private:

    Scaler(const Scaler&) = delete;
    Scaler& operator=(const Scaler&) = delete; 

    struct SwsContext* ctx_;
};

std::unique_ptr<Scaler> Scaler::create(
    const ImageFormat& src,
    const ImageFormat& dst,
    int flags)
    {
    struct SwsContext* ctx = sws_getContext(
        src.width_, src.height_, src.format_,
        dst.width_, dst.height_, dst.format_,
        flags,
        nullptr, nullptr,
        nullptr);
    return std::make_unique<Scaler>(ctx);
}

Scaler::~Scaler() {
    sws_freeContext(ctx_);
}

void Scaler::scale(
    const uint8_t* const srcSlice[], const int srcStride[],
    int srcSliceY, int srcSliceH,
    uint8_t* const dstSlice[], const int dstStride[]) {
    sws_scale(
        ctx_,
        srcSlice, srcStride,
        srcSliceY, srcSliceH,
        dstSlice, dstStride
    );
}

// ===========================
// Encoder
// ===========================

class Encoder {
public:

    static std::unique_ptr<Encoder> create(
        std::ostream& out,
        AVCodecID codec_id,
        const ImageFormat& format,
        int fps
    );

    Encoder(
        std::ostream& out,
        AVCodecContext* ctx,
        AVFrame* frame,
        const ImageFormat& format
    ):
        format_(format),
        out_(out),
        ctx_(ctx),
        frame_(frame) {}

    ~Encoder();

    const ImageFormat& format() { return format_; }
    
    AVFrame* frame() { return frame_; } 

    void next();

    void close();

private:
    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;
    
    int send_frame(AVFrame* frame);
    
    const ImageFormat format_;
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
        const ImageFormat& format,
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
    ctx->width = format.width_;
    ctx->height = format.height_;
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
    ctx->pix_fmt = format.format_;

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
    
    return std::make_unique<Encoder>(out, ctx, frame, format);
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
    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0; x < frame->width; ++x) {
            frame->data[0][y * frame->linesize[0] + x] = x + tt * 3;
        }
    }/*
    for (int y = 0; y < frame->height/2; y++) {
        for (int x = 0; x < frame->width/2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128 + y + tt * 2;
            frame->data[2][y * frame->linesize[2] + x] = 64 + x + tt * 5;
        }
    }*/
}

void make_image(int tt, std::unique_ptr<AvImage>& img) {
    const int width = img->format().width_;
    const int height = img->format().height_;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            img->data_[0][(x * 4) + (y * width * 4) + 0] = x + tt * 3;
            img->data_[0][(x * 4) + (y * width * 4) + 1] = x + tt * 3;
            img->data_[0][(x * 4) + (y * width * 4) + 2] = x + tt * 3;
        }
    }
}

void video_encode_example2(AVCodecID codec_id) {
    std::ofstream fout("test.mpg1", std::ios::out | std::ios::binary);
    
    auto rgbImage = AvImage::create(ImageFormat(352, 288, AV_PIX_FMT_RGB32), 1);
    
    auto encoder = Encoder::create(fout, codec_id, ImageFormat(352, 288, AV_PIX_FMT_YUV420P), 25);
    {
        auto scaler = Scaler::create(rgbImage->format(), encoder->format(), SWS_BILINEAR);
        for (int tt = 0; tt < 10 * 25; ++tt) {
            make_image2(tt, encoder->frame());
            make_image(tt, rgbImage);
            scaler->scale(
                rgbImage->data_, rgbImage->linesize_,
                0, rgbImage->format().height_,
                encoder->frame()->data, encoder->frame()->linesize
            );
            encoder->frame()->pts = tt;
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
//    cout << "decode begin" << endl << flush;
//    video_decode(codec_id);
//    cout << "decode end" << endl << flush;

}