#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/tab/Tab.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Tab::Tab(UIContext &ui):
		Widget(UI_SCALE), ui(ui) {}

	void Tab::renderIcon(const RendererContext &) {}

	void Tab::renderIconTexture(const RendererContext &renderers, const std::shared_ptr<Texture> &texture) {
		renderers.singleSprite.drawOnScreen(texture, RenderOptions{
			.x = 5 * UI_SCALE,
			.y = 5 * UI_SCALE,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = 5,
			.scaleY = 5,
			.invertY = false,
		});
	}

	SizeRequestMode Tab::getRequestMode() const {
		return SizeRequestMode::Expansive;
	}

	void Tab::measure(const RendererContext &, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		minimum = natural = orientation == Orientation::Vertical? for_height : for_width;
	}
}
