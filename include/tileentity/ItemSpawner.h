#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class ItemSpawner: public TileEntity {
		public:
			float chancePerTenth;
			std::vector<ItemStack> spawnables;

			ItemSpawner(const ItemSpawner &) = delete;
			ItemSpawner(ItemSpawner &&) = default;
			~ItemSpawner() override = default;

			ItemSpawner & operator=(const ItemSpawner &) = delete;
			ItemSpawner & operator=(ItemSpawner &&) = default;

			TileEntityID getID() const override { return TileEntity::ITEM_SPAWNER; }

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			void init(std::default_random_engine &) override;
			using TileEntity::init;
			void tick(Game &, float) override;
			void render(SpriteRenderer &) override;

		protected:
			ItemSpawner() = default;
			ItemSpawner(TileID id_, const Position &position_):
				TileEntity(id_, TileEntity::ITEM_SPAWNER, position_, false) {}

			friend class TileEntity;
	};
}
