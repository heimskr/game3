#include "ui/gl/module/Module.h"
#include "ui/gl/Constants.h"

namespace Game3 {
	Module::Module(float scale):
		Widget(scale) {}

	Module::Module():
		Module(UI_SCALE) {}

	void Module::reset() {}

	void Module::update() {}

	std::optional<Buffer> Module::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		return {};
	}
}
