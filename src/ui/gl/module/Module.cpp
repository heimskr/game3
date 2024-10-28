#include "ui/gl/module/Module.h"
#include "ui/gl/Constants.h"

namespace Game3 {
	Module::Module(UIContext &ui, std::weak_ptr<ClientGame> weak_game, float scale):
		ChildDependentExpandingWidget<Widget>(ui, scale), weakGame(std::move(weak_game)) {}

	Module::Module(UIContext &ui, std::weak_ptr<ClientGame> weak_game):
		Module(ui, std::move(weak_game), UI_SCALE) {}

	Module::Module(UIContext &ui, float scale):
		Module(ui, {}, scale) {}

	Module::Module(UIContext &ui):
		Module(ui, UI_SCALE) {}

	void Module::reset() {}

	void Module::update() {}

	void Module::setInventory(std::shared_ptr<ClientInventory>) {}

	std::optional<Buffer> Module::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		return {};
	}

	std::shared_ptr<InventoryModule> Module::getPrimaryInventoryModule() {
		return {};
	}

	bool Module::handleShiftClick(std::shared_ptr<Inventory>, Slot) {
		return false;
	}

	ClientGamePtr Module::getGame() const {
		if (auto game = weakGame.lock())
			return game;
		throw std::runtime_error("Couldn't lock Module's game");
	}
}
