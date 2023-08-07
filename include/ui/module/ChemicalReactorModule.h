#pragma once

#include "Types.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class ChemicalReactor;
	class InventoryTab;

	class ChemicalReactorModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/chemical_reactor"}; }

			ChemicalReactorModule(std::shared_ptr<ClientGame>, const std::any &);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void handleMessage(Agent &source, const std::string &name, Buffer &data) final;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<ChemicalReactor> reactor;
			Gtk::Label header;
			Gtk::Entry entry;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
