#include "entity/EntityFactory.h"

namespace Game3 {
	EntityFactory::EntityFactory(Identifier identifier_, decltype(function) function):
		NamedRegisterable(std::move(identifier_)), function(std::move(function)) {}

	std::shared_ptr<Entity> EntityFactory::operator()(const std::shared_ptr<Game> &game) {
		if (!function)
			throw std::logic_error("EntityFactory is missing a default function");

		return function(game);
	}

	std::shared_ptr<Entity> EntityFactory::operator()(const std::shared_ptr<Game> &game, const boost::json::value &json) {
		EntityPtr entity = (*this)(game);
		entity->absorbJSON(game, json);
		return entity;
	}
}
