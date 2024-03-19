#pragma once

#include "types/Types.h"
#include "ui/module/GeneticAnalysisModule.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class InventoryModule;
	class InventoryTab;
	class Microscope;

	class MicroscopeModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/microscope"}; }

			MicroscopeModule(ClientGamePtr, const std::any &);
			MicroscopeModule(ClientGamePtr, std::shared_ptr<Microscope>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void updateResults();
			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final { return inventoryModule; }

		private:
			ClientGamePtr game;
			std::shared_ptr<Microscope> microscope;
			std::shared_ptr<InventoryModule> inventoryModule;
			GeneticAnalysisModule geneticAnalysisModule;
			Gtk::Label header;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
