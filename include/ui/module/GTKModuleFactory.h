#pragma once

#include "types/Types.h"
#include "ui/module/GTKModule.h"
#include "registry/Registerable.h"

#include <any>
#include <functional>
#include <memory>

namespace Game3 {
	class Game;

	class GTKModuleFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<GTKModule>(const std::shared_ptr<ClientGame> &, const std::any &)> function;

		public:
			GTKModuleFactory(Identifier, decltype(function));

			std::shared_ptr<GTKModule> operator()(const std::shared_ptr<ClientGame> &, const std::any &) const;

			template <typename T>
			static GTKModuleFactory create(const Identifier &id = T::ID()) {
				return {id, [](const std::shared_ptr<ClientGame> &game, const std::any &argument) {
					return std::make_shared<T>(game, argument);
				}};
			}
	};
}
