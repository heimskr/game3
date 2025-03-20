#include "fluid/Lava.h"
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

		replaceFluid<Lava>(reg);
		replaceFluid<Water>(reg);
	}
}
