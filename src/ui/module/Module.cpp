#include "ui/module/Module.h"
#include "ui/Constants.h"

namespace Game3 {
	Module::Module(UIContext &ui, float selfScale, std::weak_ptr<ClientGame> weak_game):
		ChildDependentExpandingWidget<Widget>(ui, selfScale),
		weakGame(std::move(weak_game)) {}

	Module::Module(UIContext &ui, float selfScale):
		Module(ui, selfScale, {}) {}

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
