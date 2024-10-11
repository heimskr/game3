#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class Agent;
	class Box;
	class Button;
	class FluidsModule;
	class GeneInfoModule;
	class InventoryModule;
	class Label;
	class Mutator;

	class MutatorModule: public Module {
		public:
			MutatorModule(UIContext &, const ClientGamePtr &, const std::any &);
			MutatorModule(UIContext &, const ClientGamePtr &, std::shared_ptr<Mutator>);

			static Identifier ID() { return {"base", "module/mutator"}; }

			Identifier getID() const final { return ID(); }
			void init() final;
			void reset() final;
			void update() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setInventory(std::shared_ptr<ClientInventory>) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final;

		private:
			std::shared_ptr<Mutator> mutator;
			std::shared_ptr<InventoryModule> inventoryModule;
			std::shared_ptr<FluidsModule> fluidsModule;
			std::shared_ptr<GeneInfoModule> geneInfoModule;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Label> header;
			std::shared_ptr<Button> mutateButton;
			std::shared_ptr<Box> hbox;

			void mutate();
	};
}
