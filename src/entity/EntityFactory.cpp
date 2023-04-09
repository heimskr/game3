#include "entity/EntityFactory.h"

namespace Game3 {
	EntityFactory::EntityFactory(Identifier identifier_, decltype(function) function_):
		NamedRegisterable(std::move(identifier_)), function(std::move(function_)) {}

	std::shared_ptr<Entity> EntityFactory::operator()(Game &game, const nlohmann::json &json) {
		if (!function)
			throw std::logic_error("EntityFactory is missing a function");

		return function(game, json);
	}
}
