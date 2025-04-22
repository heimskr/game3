#pragma once

#include "types/Types.h"
#include "minigame/Minigame.h"
#include "registry/Registerable.h"

#include <any>
#include <functional>
#include <memory>

namespace Game3 {
	class Game;
	class UIContext;

	class MinigameFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<Minigame>(const std::shared_ptr<ClientGame> &, const std::any &)> function;
			static UIContext & getUIContext(ClientGame &);

		public:
			std::string gameName;

			MinigameFactory(Identifier, std::string gameName, decltype(function));

			std::shared_ptr<Minigame> operator()(const std::shared_ptr<ClientGame> &, const std::any &) const;

			template <typename T>
			static MinigameFactory create(const Identifier &id = T::ID()) {
				return {id, T::GameName(), [](const std::shared_ptr<ClientGame> &game, const std::any &) {
					auto out = std::make_shared<T>(getUIContext(*game), 1);
					out->init();
					return out;
				}};
			}
	};
}
