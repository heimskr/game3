#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/gl/tab/Tab.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/GameUI.h"
#include "ui/Window.h"

namespace Game3 {
	Tab::Tab(UIContext &ui, float selfScale):
		Widget(ui, selfScale) {}

	void Tab::renderIcon(const RendererContext &) {}

	SizeRequestMode Tab::getRequestMode() const {
		return SizeRequestMode::Expansive;
	}

	void Tab::measure(const RendererContext &, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		minimum = natural = orientation == Orientation::Vertical? for_height : for_width;
	}

	void Tab::renderIconTexture(const RendererContext &renderers, const std::shared_ptr<Texture> &texture) {
		const auto scale = getScale();

		renderers.singleSprite.drawOnScreen(texture, RenderOptions{
			.x = 5 * scale,
			.y = 5 * scale,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = 5 * scale / 8,
			.scaleY = 5 * scale / 8,
			.invertY = false,
		});
	}

	bool Tab::isActive() const {
		if (auto game_ui = ui.window.getUI<GameUI>()) {
			return game_ui->getOmniDialog()->activeTab.get() == this;
		}

		return false;
	}
}
