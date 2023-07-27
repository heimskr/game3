#pragma once

#include "Types.h"
#include "ui/module/Module.h"

#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class HasFluids;
	class InventoryTab;

	class FluidLevelsModule: public Module {
		public:
			FluidLevelsModule(std::shared_ptr<ClientGame>, std::shared_ptr<HasFluids>);

			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void updateIf(const std::shared_ptr<HasFluids> &);
			// void onResize(int) final;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<HasFluids> fluidHaver;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			// int tabWidth = 0;

			void populate();
			// void rightClick(Gtk::Widget *, int click_count, Slot, double x, double y);
	};
}
