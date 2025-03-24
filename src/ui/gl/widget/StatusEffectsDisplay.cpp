#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/StatusEffectsDisplay.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr double DISPLAY_PADDING = 2; // multiplied by scale
}

namespace Game3 {
	StatusEffectsDisplay::StatusEffectsDisplay(UIContext &ui):
		Widget(ui, 1) {}

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

		const auto scale = getScale();

		const float padding = DISPLAY_PADDING * scale;
		const float subscale = scale * 0.75;

		x = width;
		y = padding;

		player->iterateStatusEffects([&](const Identifier &, const std::unique_ptr<StatusEffect> &effect) -> bool {
			if (TexturePtr texture = effect->getTexture(game)) {
				const float icon_width = texture->width * subscale;
				const float icon_height = texture->height * subscale;

				x -= icon_width + padding;

				Icon icon(ui, scale);
				icon.setFixedSize(icon_width, icon_height);
				icon.setIconTexture(std::move(texture));
				icon.setTooltipText(effect->getName());
				icon.render(renderers, x, y, icon_width, icon_height);
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
