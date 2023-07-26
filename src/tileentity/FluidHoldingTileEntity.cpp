#include "realm/Realm.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	FluidHoldingTileEntity::FluidHoldingTileEntity(HasFluids::Map map):
		HasFluids(std::move(map)) {}

	bool FluidHoldingTileEntity::empty() {
		auto lock = fluidLevels.sharedLock();
		return fluidLevels.empty();
	}

	void FluidHoldingTileEntity::fluidsUpdated() {
		increaseUpdateCounter();
	}

	void FluidHoldingTileEntity::encode(Game &, Buffer &buffer) {
		HasFluids::encode(buffer);
	}

	void FluidHoldingTileEntity::decode(Game &, Buffer &buffer) {
		HasFluids::decode(buffer);
	}
}
