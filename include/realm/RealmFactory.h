#pragma once

#include <functional>
#include <memory>

#include <nlohmann/json_fwd.hpp>

#include "Types.h"
#include "realm/Realm.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class RealmFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Realm>(Game &)> function;

		public:
			RealmFactory(Identifier, decltype(function));

			std::shared_ptr<Realm> operator()(Game &);

			template <typename T>
			static RealmFactory create(const Identifier &id = T::ID()) {
				return {id, [](Game &game) {
					return Realm::create<T>(game);
				}};
			}
	};
}
