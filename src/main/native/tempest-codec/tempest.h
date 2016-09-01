#ifndef RPULP_TEMPEST_TEMPEST_H
#define RPULP_TEMPEST_TEMPEST_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}


namespace rpulp { namespace tempest {

/**
 */
class Size {
public:

	Size(): width_(0), height_(0) {}
	
	Size(int width, int height): width_(width), height_(height) {}

	Size(const Size& that): width_(that.width_), height_(that.height_) {}
	
	Size& operator=(const Size& that) {
		this->width_ = that.width_;
		this->height_ = that.height_;
		return *this;
	}

	bool operator==(const Size& that) const {
		return this->width_ == that.width_ && this->height_ == that.height_;
	}
	
	bool operator!=(const Size& that) const {
		return !(*this == that);
	}
	
	int width() const { return width_; }
	
	void width(int ww) { this->width_ = ww; }
	
	int height() const { return height_; }
	
	void height(int hh) { height_ = hh; }	
	
private:
	int width_;
	int height_;
};


/**
 */
class ImageClip {
public:
	std::string path_;
    int duration_;

	ImageClip(const std::string& path, int duration):
        path_(path),
        duration_(duration)
    {}
	
	ImageClip(const ImageClip& that):
        path_(that.path_),
        duration_(that.duration_)
    {}

	ImageClip& operator=(const ImageClip& that) {
		this->path_ = that.path_;
        this->duration_ = that.duration_;
		return *this;
	}
};


/**
 */
class EncodeVideoParams {
public:
	std::string codecName_;
    Size size_;
    int fps_;

    EncodeVideoParams(
        const std::string& codecName,
        const Size& size,
        int fps
    ): codecName_(codecName), size_(size), fps_(fps) {}
};

/**
 */
class Ffmpeg {
public:
	Ffmpeg();
    
    void encode_video(
        const EncodeVideoParams& params,
        std::ostream& out,
        std::function<bool (int, AVFrame*)> next_frame_func);

    void load_image_into_frame(const std::string& filename, AVFrame* frame);

private:
	Ffmpeg(const Ffmpeg& that) = delete;
	Ffmpeg& operator=(const Ffmpeg& that) = delete;
};

void images_to_clip(
    const std::vector<ImageClip>& images,
    const EncodeVideoParams& params,
    const std::string& clip_path);

}}

#endif