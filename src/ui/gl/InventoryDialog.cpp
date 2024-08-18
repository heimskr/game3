#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/InventoryDialog.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr double X_FRACTION = 0.666;
	constexpr double Y_FRACTION = 0.666;
}

namespace Game3 {
	void InventoryDialog::render(UIContext &ui, RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		Rectangle rectangle = stack.getTop();
		// INFO("Hello! {}", rectangle);
		rectangle.x = rectangle.width * X_FRACTION / 2;
		rectangle.y = rectangle.height * Y_FRACTION / 2;
		rectangle.width *= X_FRACTION;
		rectangle.height *= Y_FRACTION;

		// stack.pushRelative(rectangle);
		// INFO("{}", rectangle);

		SingleSpriteRenderer &single = renderers.singleSprite;
		single.drawOnScreen(cacheTexture("resources/gui/gui_top.png", false), RenderOptions{
			.x = 64.,
			.y = 96.,
			.sizeX = 100.,
			.sizeY = -1.,
			.scaleX = 16.,
			.scaleY = 16.,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		// INFO("{}", std::filesystem::canonical("resources/gui/gui_top.png").string());
	}
}
