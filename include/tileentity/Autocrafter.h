#pragma once

#include "threading/Lockable.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	struct CraftingRecipe;

	class Autocrafter: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/autocrafter"}; }

			std::string getName() const override { return "Autocrafter"; }

			bool mayInsertItem(const ItemStack &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			void init(Game &) override;
			void tick(Game &, float) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;

			const auto & getTarget() const { return target; }

		private:
			float accumulatedTime = 0.f;
			Lockable<std::vector<std::shared_ptr<CraftingRecipe>>> cachedRecipes;
			Lockable<Identifier> target;
			Lockable<std::optional<ItemStack>> stationStack;
			Lockable<Identifier> station;

			Autocrafter();
			Autocrafter(Identifier tile_id, Position);
			Autocrafter(Position);

			void autocraft();
			bool setTarget(Identifier);
			void cacheRecipes();
			bool setStation(std::optional<ItemStack>);
			bool validateRecipe(const CraftingRecipe &) const;

			friend class TileEntity;
	};
}
