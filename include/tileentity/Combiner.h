#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"
#include "threading/Lockable.h"

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
			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			const auto & getTarget() const { return target; }

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			Game & getGame() const final;

		private:
			float accumulatedTime = 0.f;
			Slot currentSlot = 0;
			Lockable<Identifier> target;
			std::shared_ptr<CombinerRecipe> recipe;

			Combiner();
			Combiner(Identifier tile_id, Position);
			Combiner(Position);

			bool combine();
			bool setTarget(Identifier);

			friend class TileEntity;
	};
}
