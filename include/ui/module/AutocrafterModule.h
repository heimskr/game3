#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class Autocrafter;
	class ExternalInventoryModule;
	class InventoryTab;

	class AutocrafterModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/autocrafter"}; }

			AutocrafterModule(std::shared_ptr<ClientGame>, const std::any &);
			AutocrafterModule(std::shared_ptr<ClientGame>, std::shared_ptr<Autocrafter>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) override;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Autocrafter> autocrafter;
			std::unique_ptr<ExternalInventoryModule> inventoryModule;
			Gtk::Label header;
			Gtk::Entry entry;
			Gtk::Fixed fixed;
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
			void configureFixed();
	};
}
