#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class Box;
	class ChemicalReactor;
	class EnergyModule;
	class InventoryModule;
	class Label;
	class TextInput;

	class ChemicalReactorModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/chemical_reactor"}; }

			ChemicalReactorModule(UIContext &, const ClientGamePtr &, const std::any &);
			ChemicalReactorModule(UIContext &, const ClientGamePtr &, std::shared_ptr<ChemicalReactor>);

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			// std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final { return inventoryModule; }

		private:
			std::shared_ptr<ChemicalReactor> reactor;
			std::shared_ptr<InventoryModule> inventoryModule;
			std::shared_ptr<EnergyModule> energyModule;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Label> header;
			std::shared_ptr<TextInput> formulaInput;

			void populate();
			void setEquation(const std::string &);
	};
}
