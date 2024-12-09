#pragma once

#include <iostream>
#include <functional>
#include <memory>

#include <boost/json/fwd.hpp>

#include "types/Types.h"
#include "entity/Entity.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class EntityFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Entity>(const std::shared_ptr<Game> &)> function;

		public:
			EntityFactory(Identifier, decltype(function));

			std::shared_ptr<Entity> operator()(const std::shared_ptr<Game> &);
			std::shared_ptr<Entity> operator()(const std::shared_ptr<Game> &, const boost::json::value &);

			template <typename T>
			static EntityFactory create(const Identifier &id = T::ID()) {
				return {id, [](const std::shared_ptr<Game> &game) {
					return T::create(game);
				}};
			}
	};
}
