#include "entity/EntityFactory.h"

namespace Game3 {
	EntityFactory::EntityFactory(Identifier identifier_, decltype(defaultFunction) default_function, decltype(jsonFunction) json_function):
		NamedRegisterable(std::move(identifier_)), defaultFunction(std::move(default_function)), jsonFunction(std::move(json_function)) {}

	std::shared_ptr<Entity> EntityFactory::operator()(Game &game) {
		if (!defaultFunction)
			throw std::logic_error("EntityFactory is missing a default function");

		return defaultFunction(game);
	}

	std::shared_ptr<Entity> EntityFactory::operator()(Game &game, const nlohmann::json &json) {
		if (!jsonFunction)
			throw std::logic_error("EntityFactory is missing a JSON function");

		return jsonFunction(game, json);
	}
}
