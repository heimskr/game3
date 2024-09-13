#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class Autocrafter;
	class EnergyModule;
	class InventoryModule;
	class TextInput;

	class AutocrafterModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/autocrafter"}; }

			AutocrafterModule(UIContext &, const ClientGamePtr &, const std::any &);
			AutocrafterModule(UIContext &, const ClientGamePtr &, std::shared_ptr<Autocrafter>);

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			// std::shared_ptr<GTKInventoryModule> getPrimaryInventoryModule() final { return inventoryModule; }

		private:
			std::weak_ptr<ClientGame> weakGame;
			std::shared_ptr<Autocrafter> autocrafter;
			std::shared_ptr<InventoryModule> inventoryModule;
			std::shared_ptr<InventoryModule> stationInventoryModule;
			std::shared_ptr<EnergyModule> energyModule;
			std::shared_ptr<TextInput> identifierInput;

			void setTarget(const std::string &);
	};
}
