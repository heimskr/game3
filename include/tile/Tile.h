#pragma once

#include "Layer.h"
#include "registry/Registerable.h"
#include "threading/Lockable.h"
#include "types/Types.h"

#include <optional>
#include <vector>

namespace Game3 {
	class EntityFactory;
	class Game;
	class ItemStack;
	struct Place;

	class Tile: public NamedRegisterable {
		public:
			Tile(Identifier);
			virtual ~Tile() = default;

			virtual void randomTick(const Place &);
			/** Returns false to continue propagation to lower layers, true to stop it. */
			virtual bool interact(const Place &, Layer, ItemStack *used_item, Hand);

			virtual bool canSpawnMonsters(const Place &) const;

		private:
			Lockable<std::optional<std::vector<std::shared_ptr<EntityFactory>>>> monsterFactories;

			void makeMonsterFactories(Game &);
	};
}
