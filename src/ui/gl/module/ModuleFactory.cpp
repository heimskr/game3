#include "ui/gl/module/ModuleFactory.h"

namespace Game3 {
	ModuleFactory::ModuleFactory(Identifier identifier, decltype(function) function):
		NamedRegisterable(std::move(identifier)), function(std::move(function)) {}

	std::shared_ptr<Module> ModuleFactory::operator()(const std::shared_ptr<ClientGame> &game, const std::any &argument) const {
		if (!function)
			throw std::logic_error("ModuleFactory is missing a function");

		return function(game, argument);
	}
}
