#include "ui/Canvas.h"

namespace Game3 {
	Canvas::Canvas(nanogui::Widget *parent_): GLCanvas(parent_) {
		setBackgroundColor({255, 255, 255, 255});
	}
}
