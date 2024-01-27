#pragma once

#include "types/Types.h"
#include "ui/gtk/ItemSlot.h"
#include "ui/module/Module.h"

#include <any>
#include <map>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class Village;

	class VillageTradeModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/village_trade"}; }

			VillageTradeModule(std::shared_ptr<ClientGame>, const std::any &);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;

		private:
			class Row: public Gtk::Box {
				public:
					Row(const std::shared_ptr<ClientGame> &, const Identifier &resource, double amount);

					void update(double amount);

				private:
					ItemSlot itemSlot;
					Gtk::Label quantityLabel;
					Gtk::SpinButton transferAmount;
					Gtk::Button buyButton{"Buy"};
					Gtk::Button sellButton{"Sell"};
			};

			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Village> village;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			std::map<Identifier, std::unique_ptr<Row>> rows;
			Gtk::Label villageName;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
