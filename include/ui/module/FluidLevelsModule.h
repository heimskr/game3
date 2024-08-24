#pragma once

#include "types/Types.h"
#include "ui/module/GTKModule.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class HasFluids;
	class InventoryTab;

	class FluidLevelsModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/fluid_levels"}; }

			/** The std::any argument is expected to hold an AgentPtr to an Agent that extends HasFluids. */
			FluidLevelsModule(std::shared_ptr<ClientGame>, const std::any &, bool show_header = true);
			FluidLevelsModule(std::shared_ptr<ClientGame>, const AgentPtr &, bool show_header = true);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;

			void updateIf(const std::shared_ptr<HasFluids> &);

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<HasFluids> fluidHaver;
			bool showHeader{};
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
