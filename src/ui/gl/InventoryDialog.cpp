#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/InventoryDialog.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr double X_FRACTION = 0.25;
	constexpr double Y_FRACTION = 0.25;
}

namespace Game3 {
	void InventoryDialog::render(UIContext &ui, RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		Rectangle rectangle = stack.getTop();
		rectangle.x = rectangle.width * X_FRACTION / 2;
		rectangle.y = rectangle.height * Y_FRACTION / 2;
		rectangle.width *= (1. - X_FRACTION);
		rectangle.height *= (1. - Y_FRACTION);

		stack.pushRelative(rectangle);

		auto saver = renderers.getSaver();
		renderers.updateSize(rectangle.width, rectangle.height);

		drawFrame(ui, renderers, 8, false, {
			"resources/gui/gui_topleft.png",
			"resources/gui/gui_top.png",
			"resources/gui/gui_topright.png",
			"resources/gui/gui_right.png",
			"resources/gui/gui_bottomright.png",
			"resources/gui/gui_bottom.png",
			"resources/gui/gui_bottomleft.png",
			"resources/gui/gui_left.png",
		});
	}
}
