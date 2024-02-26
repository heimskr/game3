#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"
#include "threading/Lockable.h"
#include "threading/LockableSharedPtr.h"

namespace Game3 {
	struct CombinerRecipe;

	class Combiner: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/combiner"}; }

			bool mayInsertItem(const ItemStack &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Combiner"; }
			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;
			const auto & getTarget() const { return target; }

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Slot currentSlot = 0;
			Lockable<Identifier> target;
			LockableSharedPtr<CombinerRecipe> recipe;

			Combiner();
			Combiner(Identifier tile_id, Position);
			Combiner(Position);

			bool combine();
			bool setTarget(Identifier);

			friend class TileEntity;
	};
}
