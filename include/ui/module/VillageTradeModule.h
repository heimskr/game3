#pragma once

#include "types/Types.h"
#include "ui/gtk/ItemSlot.h"
#include "ui/module/GTKModule.h"

#include <any>
#include <map>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class Village;

	class VillageTradeModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/village_trade"}; }

			VillageTradeModule(std::shared_ptr<ClientGame>, const std::any &);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			bool handleShiftClick(std::shared_ptr<Inventory>, Slot) override;

		private:
			class Row: public Gtk::Box {
				public:
					Row(const std::shared_ptr<ClientGame> &, VillageID, const Item &item, double amount_);

					void setAmount(double);
					void updateLabel();
					void updateTooltips(ItemCount);

				private:
					VillageID villageID{};
					Identifier resource;
					ItemSlot itemSlot;
					double basePrice{};
					double amount{};
					Gtk::Label quantityLabel;
					Gtk::SpinButton transferAmount;
					Gtk::Button buyButton{"Buy"};

					ItemCount getCount() const;
					void buy(const std::shared_ptr<ClientGame> &, ItemCount);
					void sell(const std::shared_ptr<ClientGame> &, ItemCount);
			};

			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Village> village;
			std::map<Identifier, std::unique_ptr<Row>> rows;
			Gtk::Label villageName;
			Gtk::Label laborLabel;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Glib::RefPtr<Gtk::DropTarget> dropTarget;

			Gtk::Box sellRow{Gtk::Orientation::HORIZONTAL};
			bool sellRowShown = false;
			ItemSlot sellSlot;
			Gtk::SpinButton sellCount;
			Gtk::Button sellButton{"Sell"};
			Gtk::Box sellLabelBox{Gtk::Orientation::VERTICAL};
			Gtk::Label totalPriceLabel;
			Gtk::Label unitPriceLabel;

			bool setSellStack(ItemStackPtr);
			void updateSell(const ItemStackPtr &);
			void sell();
			void showSell();
			void hideSell();
			void populate();
	};
}
