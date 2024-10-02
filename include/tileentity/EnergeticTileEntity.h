#pragma once

#include "game/HasEnergy.h"
#include "game/Observable.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "tileentity/TileEntity.h"

#include <functional>
#include <optional>

namespace Game3 {
	/**
	 * This class inherits TileEntity *virtually*. It doesn't call any TileEntity methods itself.
	 * Deriving classes must remember to do so in the encode and decode methods.
	 */
	class EnergeticTileEntity: public virtual TileEntity, public HasEnergy, public Observable {
		public:
			EnergeticTileEntity(EnergyAmount capacity, EnergyAmount energy = 0);

			virtual bool canInsertEnergy(EnergyAmount, Direction);
			/** Returns the amount not added. */
			virtual EnergyAmount addEnergy(EnergyAmount, Direction);
			virtual EnergyAmount extractEnergy(Direction, bool remove, EnergyAmount max_amount);
			virtual EnergyAmount extractEnergy(Direction, bool remove);

			void energyUpdated() override;
			void addObserver(const std::shared_ptr<Player> &, bool silent) override;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			std::shared_ptr<SetTileEntityEnergyPacket> makeEnergyPacket() const;

		protected:
			void broadcast(const std::shared_ptr<SetTileEntityEnergyPacket> &);
			using HasEnergy::addEnergy;
			using HasEnergy::getEnergy;
	};
}
