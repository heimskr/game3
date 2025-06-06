#pragma once

#include "command/local/LocalCommand.h"
#include "registry/Registerable.h"
#include "types/Types.h"

#include <functional>
#include <memory>

namespace Game3 {
	class Game;

	class LocalCommandFactory: public StringRegisterable {
		private:
			std::function<std::shared_ptr<LocalCommand>()> function;

		public:
			LocalCommandFactory(std::string, decltype(function));

			std::shared_ptr<LocalCommand> operator()();

			template <typename T>
			static LocalCommandFactory create(const std::string &id = T::name) {
				return {id, [] {
					return std::make_shared<T>();
				}};
			}
	};
}
