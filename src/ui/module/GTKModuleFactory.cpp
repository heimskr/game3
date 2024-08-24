#include "ui/module/GTKModuleFactory.h"

namespace Game3 {
	GTKModuleFactory::GTKModuleFactory(Identifier identifier_, decltype(function) function_):
		NamedRegisterable(std::move(identifier_)), function(std::move(function_)) {}

	std::shared_ptr<GTKModule> GTKModuleFactory::operator()(const std::shared_ptr<ClientGame> &game, const std::any &argument) const {
		if (!function)
			throw std::logic_error("GTKModuleFactory is missing a function");

		return function(game, argument);
	}
}
