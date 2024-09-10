#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class Autocrafter;
	class ClientGame;
	class EnergyModule;
	class InventoryModule;

	class AutocrafterModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/autocrafter"}; }

			AutocrafterModule(std::shared_ptr<ClientGame>, const std::any &);
			AutocrafterModule(std::shared_ptr<ClientGame>, std::shared_ptr<Autocrafter>);

		private:
			std::weak_ptr<ClientGame> weakGame;
			std::shared_ptr<Autocrafter> autocrafter;
			std::shared_ptr<InventoryModule> inventoryModule;
			std::shared_ptr<InventoryModule> stationInventoryModule;
			std::shared_ptr<EnergyModule> energyModule;
	};
}
