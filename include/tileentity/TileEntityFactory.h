#pragma once

#include "Types.h"
#include "tileentity/TileEntity.h"
#include "registry/Registerable.h"

#include <functional>
#include <memory>

namespace Game3 {
	class Game;

	class TileEntityFactory: public NamedRegisterable {
		private:
			std::function<std::shared_ptr<TileEntity>()> function;

		public:
			TileEntityFactory(Identifier, decltype(function));

			std::shared_ptr<TileEntity> operator()();

			template <typename T>
			static TileEntityFactory create(const Identifier &id = T::ID()) {
				return {id, []() {
					return TileEntity::create<T>();
				}};
			}
	};
}
