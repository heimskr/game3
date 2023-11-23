#include "ui/module/ModuleFactory.h"

namespace Game3 {
	ModuleFactory::ModuleFactory(Identifier identifier_, decltype(function) function_):
		NamedRegisterable(std::move(identifier_)), function(std::move(function_)) {}

	std::shared_ptr<Module> ModuleFactory::operator()(const std::shared_ptr<ClientGame> &game, const std::any &argument) const {
		if (!function)
			throw std::logic_error("ModuleFactory is missing a function");

		return function(game, argument);
	}
}
