#pragma once

#include <functional>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "tileentity/TileEntity.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class TileEntityFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<TileEntity>(Game &, const nlohmann::json &)> function;

		public:
			TileEntityFactory(Identifier, decltype(function));

			std::shared_ptr<TileEntity> operator()(Game &, const nlohmann::json &);

			template <typename T>
			static TileEntityFactory create(const Identifier &id = T::ID()) {
				return {id, [](Game &game, const nlohmann::json &json) {
					return T::fromJSON(game, json);
				}};
			}
	};
}
