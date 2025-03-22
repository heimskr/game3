#include "game/ClientGame.h"
#include "item/Item.h"
#include "statuseffect/TexturedStatusEffect.h"

namespace Game3 {
	TexturedStatusEffect::TexturedStatusEffect(Identifier identifier, Identifier itemID):
		StatusEffect(std::move(identifier)),
		itemID(std::move(itemID)) {}

	std::shared_ptr<Texture> TexturedStatusEffect::getTexture(const std::shared_ptr<ClientGame> &game) {
		if (!cachedTexture) {
			cachedTexture = game->itemRegistry->at(itemID)->getTexture(*game, {});
		}

		return cachedTexture;
	}
}
