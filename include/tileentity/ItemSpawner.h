#pragma once

#include "item/Item.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class ItemSpawner: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/item_spawner"}; }
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
			using TileEntity::init;
			void tick(Game &, float) override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			ItemSpawner() = default;
			ItemSpawner(Position position_, float chance_per_tenth, std::vector<ItemStack> spawnables_);

			friend class TileEntity;
	};
}
