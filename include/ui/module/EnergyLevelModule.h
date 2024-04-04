#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class HasEnergy;

	class EnergyLevelModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/energy_level"}; }

			/** The std::any argument is expected to hold an AgentPtr to an Agent that extends HasEnergy. */
			EnergyLevelModule(std::shared_ptr<ClientGame>, const std::any &, bool show_header = true);
			EnergyLevelModule(std::shared_ptr<ClientGame>, const AgentPtr &, bool show_header = true);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;

			void updateIf(const std::shared_ptr<HasEnergy> &);

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<HasEnergy> energyHaver;
			Gtk::Label headerLabel{"???"};
			Gtk::Label energyLabel{"Energy"};
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};
			Gtk::ProgressBar bar;

			void populate();
	};
}
