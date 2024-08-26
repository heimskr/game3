#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/tab/Tab.h"
#include "ui/gl/Constants.h"

namespace Game3 {
	Tab::Tab(UIContext &ui):
		ui(ui) {}

	void Tab::renderIconTexture(RendererContext &renderers, const std::shared_ptr<Texture> &texture) {
		renderers.singleSprite.drawOnScreen(texture, RenderOptions{
			.x = 0,
			.y = 0,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = SCALE,
			.scaleY = SCALE,
		});
	}

	void Tab::renderIcon(RendererContext &) {}

	void Tab::click(int, int) {}

	void Tab::dragStart(int, int) {}

	void Tab::dragEnd(int, int) {}
}
