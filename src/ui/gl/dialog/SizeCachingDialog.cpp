#include "ui/gl/dialog/SizeCachingDialog.h"

namespace Game3 {
	void SizeCachingDialog::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		Dialog::measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	void SizeCachingDialog::rescale(float new_scale) {
		SizeCacher::rescale(new_scale);
		Dialog::rescale(new_scale);
	}
}
