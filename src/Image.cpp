#include <iostream>

#include <nanogui/opengl.h>

#include "Image.h"

namespace Game3 {
	Image::Image(NVGcontext *context, const std::filesystem::path &path, int flags) {
		initialize(context, path, flags);
	}

	Image::~Image() {
		if (nvg_ != -1)
			nvgDeleteImage(context_, nvg_);
	}

	void Image::initialize(NVGcontext *context, const std::filesystem::path &path, int flags) {
		if (nvg_ != -1) {
			nvgDeleteImage(context_, nvg_);
		}

		context_ = context;
		nvg_ = nvgCreateImage(context, path.c_str(), flags);
		nvgImageSize(context, nvg_, &width_, &height_);
	}

	void Image::draw(float x, float y, float scale, float angle, float alpha) {
		if (nvg_ != -1)
			draw(x, y, 0, 0, width_, height_, scale, angle, alpha);
	}

	void Image::draw(float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (nvg_ == -1)
			return;
		nvgSave(context_);
		auto pattern = nvgImagePattern(context_, x + x_offset, y + y_offset, width_ * scale, height_ * scale, angle, nvg_, alpha);
		nvgBeginPath(context_);
		if (size_x < 0)
			size_x = width_;
		if (size_y < 0)
			size_y = height_;
		nvgRect(context_, x, y, size_x * scale, size_y * scale);
		nvgFillPaint(context_, pattern);
		nvgFill(context_);
		nvgRestore(context_);
	}
}
