#pragma once

#include "chemskr/Chemskr.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Disruptor: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/disruptor"}; }

			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Disruptor"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Disruptor();
			Disruptor(Identifier tile_id, Position);
			Disruptor(Position);

			bool react();
			const std::map<std::string, size_t> & getAtomCounts(std::unique_lock<DefaultMutex> &, const std::string &formula);

			static Lockable<std::map<std::string, std::map<std::string, size_t>>> atomCounts;

		friend class TileEntity;
	};
}
