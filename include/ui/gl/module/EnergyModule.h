#pragma once

#include "game/InventoryGetter.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class ClientGame;
	class ClientInventory;
	class HasEnergy;
	class ProgressBar;

	class EnergyModule: public Module {
		public:
			EnergyModule(std::shared_ptr<ClientGame>, const std::any &, bool show_header = true);
			EnergyModule(std::shared_ptr<ClientGame>, const AgentPtr &, bool show_header = true);

			static Identifier ID() { return {"base", "module/energy_level"}; }

			Identifier getID() const final { return ID(); }
			void init(UIContext &) final;

			using Module::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

		private:
			std::shared_ptr<HasEnergy> energyHaver;
			std::shared_ptr<ProgressBar> bar;
			bool showHeader{};
	};
}
