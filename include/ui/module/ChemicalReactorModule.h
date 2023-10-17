#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class ChemicalReactor;
	class ExternalInventoryModule;
	class InventoryTab;

	class ChemicalReactorModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/chemical_reactor"}; }

			ChemicalReactorModule(std::shared_ptr<ClientGame>, const std::any &);
			ChemicalReactorModule(std::shared_ptr<ClientGame>, std::shared_ptr<ChemicalReactor>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) override;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<ChemicalReactor> reactor;
			std::unique_ptr<ExternalInventoryModule> inventoryModule;
			Gtk::Label header;
			Gtk::Entry entry;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
