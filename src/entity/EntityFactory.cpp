#include "entity/EntityFactory.h"

namespace Game3 {
	EntityFactory::EntityFactory(Identifier identifier_, decltype(defaultFunction) default_function):
		NamedRegisterable(std::move(identifier_)), defaultFunction(std::move(default_function)) {}

	std::shared_ptr<Entity> EntityFactory::operator()(const std::shared_ptr<Game> &game) {
		if (!defaultFunction)
			throw std::logic_error("EntityFactory is missing a default function");

		return defaultFunction(game);
	}

	std::shared_ptr<Entity> EntityFactory::operator()(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
		EntityPtr entity = defaultFunction(game);
		entity->absorbJSON(game, json);
		return entity;
	}
}
