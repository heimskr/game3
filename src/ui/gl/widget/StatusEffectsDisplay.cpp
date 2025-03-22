#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/gl/widget/StatusEffectsDisplay.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr double DISPLAY_PADDING = 2; // multiplied by scale
}

namespace Game3 {
	StatusEffectsDisplay::StatusEffectsDisplay(UIContext &ui):
		Widget(ui, UI_SCALE) {}

	void StatusEffectsDisplay::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(renderers, x, y, width, height);

		ClientGamePtr game = ui.getGame();
		if (!game) {
			return;
		}

		ClientPlayerPtr player = game->getPlayer();
		if (!player) {
			return;
		}

		const double padding = DISPLAY_PADDING * scale;
		const double subscale = scale * 0.75;

		x = width;
		y = padding;

		player->iterateStatusEffects([&](const Identifier &identifier, const std::unique_ptr<StatusEffect> &effect) -> bool {
			if (TexturePtr texture = effect->getTexture(game)) {
				x -= texture->width * subscale + padding;
				renderers.singleSprite.drawOnScreen(texture, RenderOptions{
					.x = x,
					.y = y,
					.sizeX = -1.0,
					.sizeY = -1.0,
					.scaleX = subscale,
					.scaleY = subscale,
					.invertY = false,
				});
			}
			return false;
		});
	}

	SizeRequestMode StatusEffectsDisplay::getRequestMode() const {
		return SizeRequestMode::Expansive;
	}

	void StatusEffectsDisplay::measure(const RendererContext &, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		minimum = natural = measure_orientation == Orientation::Horizontal? for_width : for_height;
	}
}
