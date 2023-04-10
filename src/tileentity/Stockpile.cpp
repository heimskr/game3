#include <iostream>

#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Stockpile.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Stockpile::Stockpile(Identifier tilename, Position position_, const Texture &texture_):
	Chest(std::move(tilename), std::move(position_), "Stockpile", texture_) {
		tileEntityID = ID();
	}

	void Stockpile::toJSON(nlohmann::json &json) const {
		Chest::toJSON(json);
	}

	bool Stockpile::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto keep = std::dynamic_pointer_cast<Keep>(getRealm());
		if (!keep)
			throw std::runtime_error("Stockpile must be placed inside a Keep realm");
		std::cout << "Keep: money = " << keep->money << ", greed = " << keep->greed << '\n';
		Chest::onInteractNextTo(player);
		return true;
	}

	void Stockpile::absorbJSON(Game &game, const nlohmann::json &json) {
		Chest::absorbJSON(game, json);
	}
}
