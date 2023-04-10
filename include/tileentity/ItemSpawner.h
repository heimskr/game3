#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class ItemSpawner: public TileEntity {
		public:
			// TODO: different chance per spawnable
			// TODO: spawn time chosen between upper bound and lower bound
			float chancePerTenth;
			std::vector<ItemStack> spawnables;

			ItemSpawner(const ItemSpawner &) = delete;
			ItemSpawner(ItemSpawner &&) = default;
			~ItemSpawner() override = default;

			ItemSpawner & operator=(const ItemSpawner &) = delete;
			ItemSpawner & operator=(ItemSpawner &&) = default;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void init(Game &, std::default_random_engine &) override;
			using TileEntity::init;
			void tick(Game &, float) override;
			void render(SpriteRenderer &) override;

		protected:
			ItemSpawner() = default;
			ItemSpawner(Position position_, float chance_per_tenth, std::vector<ItemStack> spawnables_);

			friend class TileEntity;
	};
}
