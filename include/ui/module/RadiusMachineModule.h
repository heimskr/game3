#pragma once

#include "mixin/HasRadius.h"
#include "types/Types.h"
#include "ui/module/Module.h"

namespace Game3 {
	class EnergyModule;
	class FluidsModule;
	class InventoryModule;

	class RadiusMachineModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/radius_machine"}; }

			RadiusMachineModule(UIContext &, float selfScale, const ClientGamePtr &, const std::any &);
			RadiusMachineModule(UIContext &, float selfScale, const ClientGamePtr &, TileEntityPtr);

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final;

		private:
			TileEntityPtr machine;
			std::shared_ptr<InventoryModule> inventoryModule;
			std::shared_ptr<EnergyModule> energyModule;
			std::shared_ptr<FluidsModule> fluidsModule;

			void setRadius(Radius);
	};
}
