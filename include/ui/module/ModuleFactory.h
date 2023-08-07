#pragma once

#include "Types.h"
#include "ui/module/Module.h"
#include "registry/Registerable.h"

#include <functional>
#include <memory>

namespace Game3 {
	class Game;

	class ModuleFactory: public NamedRegisterable {
		private:
			std::function<std::unique_ptr<Module>(const std::shared_ptr<ClientGame> &, const std::any &)> function;

		public:
			ModuleFactory(Identifier, decltype(function));

			std::unique_ptr<Module> operator()(const std::shared_ptr<ClientGame> &, const std::any &) const;

			template <typename T>
			static ModuleFactory create(const Identifier &id = T::ID()) {
				return {id, [](const std::shared_ptr<ClientGame> &game, const std::any &argument) {
					return std::make_unique<T>(game, argument);
				}};
			}
	};
}
