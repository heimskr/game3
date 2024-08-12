#pragma once

#include "threading/Lockable.h"
#include "threading/LockableSharedPtr.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Texture;
	struct CraftingRecipe;

	class Autocrafter: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/autocrafter"}; }

			std::string getName() const override { return "Autocrafter"; }

			const std::shared_ptr<Inventory> & getInventory(InventoryID) const override;
			using InventoriedTileEntity::setInventory;
			void setInventory(std::shared_ptr<Inventory>, InventoryID) override;
			InventoryID getInventoryCount() const override { return 2; }

			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			void init(Game &) override;
			void tick(const TickArgs &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;

			void render(SpriteRenderer &) override;
			void renderUpper(SpriteRenderer &) override;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;

			const auto & getTarget() const { return target; }
			bool setTarget(Identifier);

			const auto & getStationInventory() const { return stationInventory; }

		private:
			Lockable<std::vector<std::shared_ptr<CraftingRecipe>>> cachedRecipes;
			Lockable<Identifier> target;
			Lockable<std::shared_ptr<Inventory>> stationInventory;
			Lockable<Identifier> station;
			TileID cachedArmLower = -1;
			TileID cachedArmUpper = -1;
			LockableSharedPtr<Texture> stationTexture;
			float stationXOffset{};
			float stationYOffset{};
			float stationSizeX{};
			float stationSizeY{};

			Autocrafter();
			Autocrafter(Identifier tile_id, Position);
			Autocrafter(Position);

			void autocraft();
			void cacheRecipes();
			bool stationSet();
			void setStationTexture(const ItemStackPtr &);
			void resetStationTexture();
			bool validateRecipe(const CraftingRecipe &) const;
			void connectStationInventory();

		friend class TileEntity;
	};
}
