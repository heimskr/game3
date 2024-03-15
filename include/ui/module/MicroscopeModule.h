#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class Microscope;
	class InventoryModule;
	class InventoryTab;

	class MicroscopeModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/microscope"}; }

			MicroscopeModule(std::shared_ptr<ClientGame>, const std::any &);
			MicroscopeModule(std::shared_ptr<ClientGame>, std::shared_ptr<Microscope>);

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
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Microscope> microscope;
			std::shared_ptr<InventoryModule> inventoryModule;
			Gtk::Label header;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::Box resultsBox{Gtk::Orientation::VERTICAL};
			std::vector<std::unique_ptr<Gtk::Label>> resultsLabels;

			void populate();
	};
}
