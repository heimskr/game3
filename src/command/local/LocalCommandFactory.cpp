#include "command/local/LocalCommandFactory.h"

namespace Game3 {
	LocalCommandFactory::LocalCommandFactory(std::string name_, decltype(function) function_):
		StringRegisterable(std::move(name_)), function(std::move(function_)) {}

	std::shared_ptr<LocalCommand> LocalCommandFactory::operator()() {
		if (!function)
			throw std::logic_error("LocalCommandFactory is missing a function");

		return function();
	}
}
