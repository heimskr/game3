// #pragma once

// #include <memory>

// #include <nanogui/nanogui.h>

// #include "menu/Menu.h"

// namespace Game3 {
// 	class Player;

// 	class InventoryMenu: public Menu {
// 		public:
// 			std::shared_ptr<Player> player;

// 			InventoryMenu(const std::shared_ptr<Player> &player_): player(player_) {}
// 			~InventoryMenu() override;

// 			virtual void render(Game &, Canvas &, NVGcontext *) override;
// 			MenuType getType() const override { return MenuType::Inventory; }

// 		private:
// 			nanogui::Window *window = nullptr;
// 	};
// }
