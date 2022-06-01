#pragma once

#include <filesystem>

struct NVGcontext;

namespace Game3 {
	class Image {
		public:
			Image() = default;
			Image(NVGcontext *, const std::filesystem::path &, int flags = 0);

			~Image();

			void initialize(NVGcontext *, const std::filesystem::path &, int flags = 0);
			void draw(float x, float y, float angle = 0.f, float alpha = 1.f);
			void draw(float x, float y, float x_offset, float y_offset, float size_x, float size_y, float angle = 0.f, float alpha = 1.f);

			int  width() const { return width_;  }
			int height() const { return height_; }

			operator bool() const { return nvg_ != -1; }

		private:
			NVGcontext *context_ = nullptr;
			int nvg_ = -1;
			int width_;
			int height_;
	};
}
