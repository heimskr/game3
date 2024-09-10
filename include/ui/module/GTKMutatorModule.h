#pragma once

#include "types/Types.h"
#include "ui/module/GTKModule.h"
#include "ui/module/GTKGeneInfoModule.h"

#include <any>
#include <memory>
#include <string>
#include <vector>

namespace Game3 {
	class Agent;
	class FluidLevelsModule;
	class GTKInventoryModule;
	class Mutator;

	class GTKMutatorModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/mutator"}; }

			GTKMutatorModule(std::shared_ptr<ClientGame>, const std::any &);
			GTKMutatorModule(std::shared_ptr<ClientGame>, std::shared_ptr<Mutator>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			std::shared_ptr<GTKInventoryModule> getPrimaryInventoryModule() final { return inventoryModule; }

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Mutator> mutator;
			std::shared_ptr<GTKInventoryModule> inventoryModule;
			std::shared_ptr<FluidLevelsModule> fluidsModule;
			GTKGeneInfoModule GTKGeneInfoModule;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::Label header;
			Gtk::Button mutateButton{"Mutate"};
			Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};

			void mutate();
	};
}
