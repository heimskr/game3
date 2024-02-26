#pragma once

#include <iostream>
#include <functional>
#include <memory>

#include <nlohmann/json_fwd.hpp>

#include "types/Types.h"
#include "entity/Entity.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class EntityFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Entity>(const std::shared_ptr<Game> &)> defaultFunction;
			std::function<std::shared_ptr<Entity>(const std::shared_ptr<Game> &, const nlohmann::json &)> jsonFunction;

		public:
			EntityFactory(Identifier, decltype(defaultFunction), decltype(jsonFunction));

			std::shared_ptr<Entity> operator()(const std::shared_ptr<Game> &);
			std::shared_ptr<Entity> operator()(const std::shared_ptr<Game> &, const nlohmann::json &);

			template <typename T>
			static EntityFactory create(const Identifier &id = T::ID()) {
				return {id, [](const std::shared_ptr<Game> &game) {
					return T::create(game);
				}, [](const std::shared_ptr<Game> &game, const nlohmann::json &json) {
					return T::fromJSON(game, json);
				}};
			}
	};
}
