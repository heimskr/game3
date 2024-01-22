#pragma once

#include "chemskr/Chemskr.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class ChemicalReactor: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/chemical_reactor"}; }

			constexpr static float ENERGY_CAPACITY = 100'000;
			constexpr static ItemCount INPUT_CAPACITY  = 5;
			constexpr static ItemCount OUTPUT_CAPACITY = 5;
			constexpr static EnergyAmount ENERGY_PER_ATOM = 100;

			bool mayInsertItem(const ItemStack &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Chemical Reactor"; }
			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			Game & getGame() const final;

			std::string getEquation();
			/** Returns whether the equation was actually set (i.e., whether the equation was valid and balanced). */
			bool setEquation(std::string);
			bool hasEquation();

		private:
			Lockable<std::optional<Chemskr::Equation>> equation;
			Lockable<std::unordered_map<std::string, size_t>> reactants;
			Lockable<std::unordered_map<std::string, size_t>> products;

			ChemicalReactor();
			ChemicalReactor(Identifier tile_id, Position);
			ChemicalReactor(Position);
			bool react();
			void fillReactants();
			void fillProducts();

			friend class TileEntity;
	};
}
