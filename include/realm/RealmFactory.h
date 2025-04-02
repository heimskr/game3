#pragma once

#include <functional>
#include <memory>

#include <boost/json/fwd.hpp>

#include "types/Types.h"
#include "realm/Realm.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class RealmFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Realm>(const std::shared_ptr<Game> &)> function;

		public:
			RealmFactory(Identifier, decltype(function));

			std::shared_ptr<Realm> operator()(const std::shared_ptr<Game> &);

			template <typename T>
			static RealmFactory create(const Identifier &id = T::ID()) {
				return {id, [](const std::shared_ptr<Game> &game) {
					return Realm::create<T>(game);
				}};
			}
	};
}
