#include "ui/gl/module/Module.h"

namespace Game3 {
	void Module::reset() {}
	void Module::update() {}

	std::optional<Buffer> Module::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		return {};
	}
}
