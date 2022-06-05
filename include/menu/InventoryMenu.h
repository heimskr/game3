#pragma once

#include <memory>

#include "menu/Menu.h"

namespace Game3 {
	class Player;

	struct InventoryMenu: Menu {
		std::shared_ptr<Player> player;
		InventoryMenu(const std::shared_ptr<Player> &player_): player(player_) {}
		~InventoryMenu() override = default;
		virtual void render(Game &, Canvas &, NVGcontext *) override;
		MenuType getType() const override { return MenuType::Inventory; }
	};
}
