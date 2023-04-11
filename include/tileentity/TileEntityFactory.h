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
			std::function<std::shared_ptr<TileEntity>(Game &)> function;

		public:
			TileEntityFactory(Identifier, decltype(function));

			std::shared_ptr<TileEntity> operator()(Game &);

			template <typename T>
			static TileEntityFactory create(const Identifier &id = T::ID()) {
				return {id, [](Game &game) {
					return TileEntity::create<T>(game);
				}};
			}
	};
}
