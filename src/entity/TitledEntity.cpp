#include "entity/TitledEntity.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	void TitledEntity::renderUpper(const RendererContext &renderers) {
		GamePtr game = getGame();
		TextRenderer &text = renderers.text;
		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();
		const bool show_message = lastMessageAge.load() < getMaxMessageAge(game->toClient());

		const float title_offset = getTitleVerticalOffset();

		if (show_message) {
			auto lock = lastMessage.sharedLock();
			renderers.text.drawOnMap(lastMessage.getBase(), {
				.x = static_cast<float>(column) + x + .5,
				.y = static_cast<float>(row) + y - z + title_offset - .25,
				.scaleX = .75,
				.scaleY = .75,
				.align = TextAlign::Center,
			});
		}

		text.drawOnMap(getDisplayName(), {
			.x = float(column) + x + .5,
			.y = float(row) + y - z + title_offset - (show_message? 1 : 0),
			.align = TextAlign::Center,
		});
	}

	float TitledEntity::getTitleVerticalOffset() const {
		return 0;
	}

	Tick TitledEntity::getMaxMessageAge(ClientGame &game) const {
		return 7 * game.getSettings().tickFrequency;
	}
}
