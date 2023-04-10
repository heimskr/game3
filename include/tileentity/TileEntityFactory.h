#pragma once

#include <functional>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "tileentity/TileEntity.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class TileEntityFactory: NamedRegisterable {
		private:
			std::function<std::shared_ptr<TileEntity>(Game &, const nlohmann::json &)> function;

		public:
			TileEntityFactory(Identifier, decltype(function));

			std::shared_ptr<TileEntity> operator()(Game &, const nlohmann::json &);

			inline const Identifier & getIdentifier() const { return identifier; }

			template <typename T>
			static TileEntityFactory create() {
				TileEntityFactory out(T::ID());
				out.function = [](Game &game, const nlohmann::json &json) {
					return TileEntity::create<T>(game, json);
				};
				return out;
			}
	};
}
