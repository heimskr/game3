#pragma once

#include "types/DirectedPlace.h"
#include "types/Types.h"
#include "ui/gtk/Util.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <optional>
#include <vector>

namespace Game3 {
	class Agent;
	class HasFluids;
	class InventoryTab;
	class ItemFilter;

	class ItemFilterModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/item_filters"}; }

			ItemFilterModule(std::shared_ptr<ClientGame>, const std::any &);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;

		private:
			std::shared_ptr<ClientGame> game;
			DirectedPlace place;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			ItemFilter *filter = nullptr;

			void populate();
	};
}
