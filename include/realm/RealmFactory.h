#pragma once

#include <functional>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "realm/Realm.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class RealmFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Realm>(Game &, const nlohmann::json &)> function;

		public:
			RealmFactory(Identifier, decltype(function));

			std::shared_ptr<Realm> operator()(Game &, const nlohmann::json &);

			template <typename T>
			static RealmFactory create(const Identifier &id = T::ID()) {
				return {id, [](Game &game, const nlohmann::json &json) {
					return T::fromJSON(game, json);
				}};
			}
	};
}
