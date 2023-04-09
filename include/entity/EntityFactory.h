#pragma once

#include <functional>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "entity/Entity.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Entity;
	class Game;

	class EntityFactory: NamedRegisterable {
		private:
			std::function<std::shared_ptr<Entity>(Game &, const nlohmann::json &)> function;

		public:
			EntityFactory(Identifier, decltype(function));

			std::shared_ptr<Entity> operator()(Game &, const nlohmann::json &);

			template <typename T>
			static EntityFactory create() {
				EntityFactory out(T::ID());
				out.function = [](Game &game, const nlohmann::json &json) {
					return Entity::create<T>(game, json);
				};
				return out;
			}
	};
}
