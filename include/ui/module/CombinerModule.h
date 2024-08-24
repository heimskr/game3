#pragma once

#include "types/Types.h"
#include "ui/module/GTKModule.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class Combiner;
	class EnergyLevelModule;
	class GTKInventoryModule;

	class CombinerModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/combiner"}; }

			CombinerModule(std::shared_ptr<ClientGame>, const std::any &);
			CombinerModule(std::shared_ptr<ClientGame>, std::shared_ptr<Combiner>);

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
			std::shared_ptr<Combiner> combiner;
			std::shared_ptr<GTKInventoryModule> inventoryModule;
			std::shared_ptr<EnergyLevelModule> energyModule;
			Gtk::Label header;
			Gtk::Entry entry;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Glib::RefPtr<Gtk::EntryCompletion> completion;

			struct ModelColumns: Gtk::TreeModelColumnRecord {
				ModelColumns() {
					add(itemName);
				}

				Gtk::TreeModelColumn<Glib::ustring> itemName;
			};

			ModelColumns columns;
			Glib::RefPtr<Gtk::ListStore> store;

			void populate();
			void setTarget(const std::string &);
	};
}
