#pragma once

#include <iostream>
#include <functional>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "entity/Entity.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class EntityFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Entity>(Game &, const nlohmann::json &)> function;

		public:
			EntityFactory(Identifier, decltype(function));

			std::shared_ptr<Entity> operator()(Game &, const nlohmann::json &);

			template <typename T>
			static EntityFactory create(const Identifier &id = T::ID()) {
				return {id, [](Game &game, const nlohmann::json &json) {
					return T::fromJSON(game, json);
				}};
			}
	};
}
