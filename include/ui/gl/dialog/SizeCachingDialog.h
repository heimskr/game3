#pragma once

#include "ui/gl/dialog/Dialog.h"
#include "ui/gl/SizeCacher.h"

namespace Game3 {
	class SizeCachingDialog: public Dialog, protected SizeCacher {
		public:
			using Dialog::Dialog;

			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;
			void rescale(float new_scale) override;
	};
}
