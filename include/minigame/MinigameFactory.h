#pragma once

#include "types/Types.h"
#include "minigame/Minigame.h"
#include "registry/Registerable.h"

#include <any>
#include <functional>
#include <memory>

namespace Game3 {
	class Game;

	class MinigameFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Minigame>(const std::shared_ptr<ClientGame> &, const std::any &)> function;

		public:
			MinigameFactory(Identifier, decltype(function));

			std::shared_ptr<Minigame> operator()(const std::shared_ptr<ClientGame> &, const std::any &) const;

			template <typename T>
			static MinigameFactory create(const Identifier &id = T::ID()) {
				return {id, [](const std::shared_ptr<ClientGame> &, const std::any &) {
					return std::make_shared<T>();
				}};
			}
	};
}
