#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Stockpile.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Stockpile::Stockpile(Identifier tilename, const Position &position_):
	Chest(std::move(tilename), position_, "Stockpile") {
		tileEntityID = ID();
	}

	bool Stockpile::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers modifiers, ItemStack *used_item, Hand hand) {
		auto keep = std::dynamic_pointer_cast<Keep>(getRealm());
		if (!keep)
			throw std::runtime_error("Stockpile must be placed inside a Keep realm");
		std::cout << "Keep: money = " << keep->money << ", greed = " << keep->greed << '\n';
		Chest::onInteractNextTo(player, modifiers, used_item, hand);
		return true;
	}
}
