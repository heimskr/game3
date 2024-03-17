#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"
#include "ui/module/MultiModule.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class InventoryModule;
	class InventoryTab;
	class Sequencer;

	class SequencerModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/sequencer"}; }

			SequencerModule(std::shared_ptr<ClientGame>, const std::any &);
			SequencerModule(std::shared_ptr<ClientGame>, std::shared_ptr<Sequencer>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Sequencer> sequencer;
			std::shared_ptr<MultiModule<Substance::Item, Substance::Energy>> multiModule;
			Gtk::Label header;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;

			void populate();
	};
}
