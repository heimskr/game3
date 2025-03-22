#pragma once

#include "fluid/Fluid.h"
#include "registry/Registerable.h"
#include "threading/Lockable.h"
#include "types/Layer.h"
#include "types/Types.h"

#include <optional>
#include <vector>

namespace Game3 {
	class EntityFactory;
	class Game;
	class ItemStack;
	struct FluidTile;
	struct Place;
	struct RendererContext;

	class Tile: public NamedRegisterable {
		public:
			Tile(Identifier);
			virtual ~Tile() = default;

			virtual void randomTick(const Place &);
			/** Returns false to continue propagation to lower layers, true to stop it. */
			virtual bool interact(const Place &, Layer, const ItemStackPtr &used_item, Hand);

			virtual bool canSpawnMonsters(const Place &) const;

			/** Should be between 0 and 1 (inclusive). */
			virtual float getMonsterSpawnProbability() const;

			virtual void renderStaticLighting(const Place &, Layer, const RendererContext &);

			/** Returns true iff renderStaticLighting actually does anything. */
			virtual bool hasStaticLighting() const { return false; }

			/** Returns true if something meaningful happened, or false if Realm::updateNeighbors should default to autotiling. */
			virtual bool update(const Place &, Layer) { return false; }

			virtual std::optional<FluidTile> yieldFluid(const Place &) { return {}; }

		private:
			Lockable<std::optional<std::vector<std::shared_ptr<EntityFactory>>>> monsterFactories;

			void makeMonsterFactories(const std::shared_ptr<Game> &);
	};

	using TilePtr = std::shared_ptr<Tile>;
}
