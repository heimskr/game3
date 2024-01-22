#pragma once

#include "item/Item.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class ItemSpawner: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/item_spawner"}; }
			float minimumTime{};
			float maximumTime{};
			std::vector<ItemStack> spawnables;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void tick(const TickArgs &) override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			ItemSpawner() = default;
			ItemSpawner(Position position_, float minimum_time, float maximum_time, std::vector<ItemStack> spawnables_);

			friend class TileEntity;
	};
}
