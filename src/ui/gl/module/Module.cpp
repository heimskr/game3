#include "ui/gl/module/Module.h"
#include "ui/gl/Constants.h"

namespace Game3 {
	Module::Module(UIContext &ui, float scale):
		Widget(ui, scale) {}

	Module::Module(UIContext &ui):
		Module(ui, UI_SCALE) {}

	void Module::reset() {}

	void Module::update() {}

	void Module::setInventory(std::shared_ptr<ClientInventory>) {}

	std::optional<Buffer> Module::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		return {};
	}
}
