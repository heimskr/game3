#include "fluid/Brine.h"
#include "fluid/FreshWater.h"
#include "fluid/Lava.h"
#include "fluid/Milk.h"
#include "fluid/Mutagen.h"
#include "fluid/PowderedSnow.h"
#include "fluid/Radon.h"
#include "fluid/Seawater.h"
#include "fluid/SulfuricAcid.h"
#include "fluid/Water.h"
#include "game/Game.h"

namespace Game3 {
	template <typename T>
	static bool replaceFluid(FluidRegistry &registry) {
		if (auto iter = registry.find(T::ID()); iter != registry.end()) {
			size_t id = iter->second->registryID;
			registry.at(id) = iter->second = std::make_shared<T>(std::move(*iter->second));
			return true;
		}

		return false;
	}

	void Game::addFluids() {
		auto &reg = registry<FluidRegistry>();

		replaceFluid<Brine>(reg);
		replaceFluid<Lava>(reg);
		replaceFluid<Milk>(reg);
		replaceFluid<Mutagen>(reg);
		replaceFluid<PowderedSnow>(reg);
		replaceFluid<Radon>(reg);
		replaceFluid<SulfuricAcid>(reg);
		replaceFluid<Water>(reg);
		replaceFluid<FreshWater>(reg);
		replaceFluid<Seawater>(reg);
	}
}
