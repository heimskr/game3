#pragma once

#include "pipes/ItemFilter.h"
#include "types/DirectedPlace.h"
#include "types/Types.h"
#include "ui/gtk/Util.h"
#include "ui/module/GTKModule.h"

#include <any>
#include <memory>
#include <optional>
#include <vector>

namespace Game3 {
	class Agent;
	class HasFluids;
	class ItemFilter;
	class Pipe;

	class ItemFilterModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/item_filters"}; }

			ItemFilterModule(std::shared_ptr<ClientGame>, const std::any &);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			bool handleShiftClick(std::shared_ptr<Inventory> source_inventory, Slot source_slot) final;

		private:
			std::shared_ptr<ClientGame> game;
			DirectedPlace place;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			std::shared_ptr<Pipe> pipe;
			std::shared_ptr<ItemFilter> filter;

			Gtk::Box topBox{Gtk::Orientation::HORIZONTAL};
			Gtk::Fixed fixed;
			Gtk::Button copyButton;
			Gtk::Button pasteButton;

			Gtk::Label modeLabel{"Whitelist"};
			Gtk::Label strictLabel{"Strict"};
			Gtk::Switch modeSwitch;
			Gtk::Switch strictSwitch;
			Gtk::Box modeHbox{Gtk::Orientation::HORIZONTAL};
			Gtk::Box strictHbox{Gtk::Orientation::HORIZONTAL};
			Gtk::Box switchesHbox{Gtk::Orientation::HORIZONTAL};

			void setMode(bool allow);
			void setStrict(bool strict);
			void upload(ItemFilterPtr = {});
			bool setFilter();
			bool saveFilter();
			void populate(ItemFilterPtr = {});

			void addHbox(const Identifier &, const ItemFilter::Config &);
			std::unique_ptr<Gtk::Image> makeImage(ItemStack &);
			std::unique_ptr<Gtk::Label> makeLabel(const ItemStack &);
			std::unique_ptr<Gtk::Button> makeComparator(const Identifier &, const ItemFilter::Config &);
			std::unique_ptr<Gtk::SpinButton> makeThreshold(const Identifier &, const ItemFilter::Config &);
			std::unique_ptr<Gtk::Button> makeButton(const ItemStackPtr &);
	};
}
