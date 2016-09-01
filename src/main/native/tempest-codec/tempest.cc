#include <iostream>
#include <fstream>
#include <stdexcept>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>

int ff_load_image(uint8_t *data[4], int linesize[4], int *w, int *h, enum AVPixelFormat *pix_fmt, const char *filename, void *log_ctx);
}

#include "tempest.h"
#include "util.h"


using namespace std;

namespace rpulp { namespace tempest {

// ==============
// Ffmpeg
// ==============

Ffmpeg::Ffmpeg()
{
    avcodec_register_all();
}

static bool video_encode_receive_packets(
    std::ostream& out,
    AVCodecContext* codec_ctx,
    AVPacket* packet)
{
    for (int ii = 0; ii < 1000 * 1000; ++ii) {
        av_init_packet(packet);
        packet->data = nullptr;    // packet data will be allocated by the encoder
        packet->size = 0;
        const int res = avcodec_receive_packet(codec_ctx, packet);
        switch (res) {
        case 0:
            out.write(reinterpret_cast<char*>(packet->data), packet->size);
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

static bool video_encode_send_frame(
    std::ostream& out,
    AVCodecContext* codec_ctx,
    AVFrame* frame,
    AVPacket* packet)
{
    for(int ii = 0; ii < 1000; ++ii) {
        const int res = avcodec_send_frame(codec_ctx, frame);
        switch (res) {
        case 0:
            return true;
        case AVERROR(EAGAIN):
            // input is not accepted right now - the frame must be
            // resent after trying to read output packets
            if (video_encode_receive_packets(out, codec_ctx, packet)) {
                break;
            } else {
                //EOS
                return false;
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

static void video_encode_loop(
    std::ostream& out,
    std::function<bool (int, AVFrame*)> next_frame_func,
    AVCodecContext* codec_ctx,
    AVFrame* frame)
{
    AVPacket packet;
    int tt = -1;
    while (next_frame_func(++tt, frame)) {
        video_encode_send_frame(out, codec_ctx, frame, &packet);
    }
    video_encode_send_frame(out, codec_ctx, nullptr, &packet);
}

void Ffmpeg::encode_video(
    const EncodeVideoParams& params,
    std::ostream& out,
    std::function<bool (int, AVFrame*)> next_frame_func)
{    
    AVCodec* codec = avcodec_find_encoder_by_name(params.codecName_.c_str());
    if (!codec) {
        throw std::runtime_error("codec not found");
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (codec_ctx == nullptr) {
        throw std::runtime_error("avcodec_alloc_context3 failed");
    }

    /* put sample parameters */
    codec_ctx->bit_rate = 400000;
    /* resolution must be a multiple of two */
    codec_ctx->width = params.size_.width();
    codec_ctx->height = params.size_.height();
    /* frames per second */
    codec_ctx->time_base = (AVRational){1, params.fps_};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    //FIXME
    /*
    if (codec_id == AV_CODEC_ID_H264) {
        av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);
    }
    */
    
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        throw std::runtime_error("avcodec_open2 failed");
    }

    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) {
        throw std::runtime_error("av_frame_alloc failed");
    }
    frame->format = codec_ctx->pix_fmt;
    frame->width  = codec_ctx->width;
    frame->height = codec_ctx->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    {
        int nbytes = av_image_alloc(frame->data, frame->linesize, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, 32);
        if (nbytes < 0) {
            throw std::runtime_error("av_image_alloc failed");
        }
        cout << "av_image_alloc() = " << nbytes << endl;
    }

    video_encode_loop(out, next_frame_func, codec_ctx, frame);
    
    /* add sequence end code to have a real MPEG file */
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    out.write(reinterpret_cast<char*>(endcode), sizeof(endcode));

    avcodec_close(codec_ctx);
    av_free(codec_ctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
}


void Ffmpeg::load_image_into_frame(const string& filename, AVFrame* frame)
{
    uint8_t* image_data[4];
    int linesize[4];
    int img_width;
    int img_height;
    enum AVPixelFormat img_fmt;

    {
        int res = ff_load_image(image_data, linesize, &img_width, &img_height, &img_fmt, filename.c_str(), nullptr);
        if (res < 0) {
            throw throw_runtime_error("ff_load_image failed");
        }
    }

    SwsContext* sws_ctx = sws_getContext(
            img_width, img_height, img_fmt,
            frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
            SWS_BICUBIC, nullptr, nullptr, nullptr
    );
    if (sws_ctx == nullptr) {
        throw throw_runtime_error("sws_getContext failed");
    }

    sws_scale(sws_ctx,
              (const uint8_t * const *)image_data, linesize,
              0, frame->height,
              frame->data, frame->linesize
    );
    
    sws_freeContext(sws_ctx);
}

std::function<bool (int, AVFrame*)> images_to_frames(const vector<ImageClip>& images) {
    return [images] (int tt, AVFrame* frame) {
        return true;
    };
}

void images_to_clip(const vector<ImageClip>& images, const EncodeVideoParams& params, const string& clip_path)
{
    ofstream out(clip_path, std::ios::out | std::ios::binary);
    Ffmpeg ffmpeg;
    ffmpeg.encode_video(params, out, images_to_frames(images));
    out.close();
}

}}
