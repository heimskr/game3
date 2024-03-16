#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/HasEnergy.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
#include "ui/module/EnergyLevelModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	EnergyLevelModule::EnergyLevelModule(std::shared_ptr<ClientGame> game_, const std::any &argument, bool show_header):
	game(std::move(game_)),
	energyHaver(std::dynamic_pointer_cast<HasEnergy>(std::any_cast<AgentPtr>(argument))) {
		vbox.set_hexpand();
		headerLabel.set_margin(10);
		headerLabel.set_xalign(0.5);
		energyLabel.set_margin(5);
		bar.set_hexpand();
		bar.set_margin_end(5);
		bar.set_valign(Gtk::Align::CENTER);
		hbox.append(energyLabel);
		hbox.append(bar);
		if (show_header)
			vbox.append(headerLabel);
		vbox.append(hbox);
	}

	Gtk::Widget & EnergyLevelModule::getWidget() {
		return vbox;
	}

	void EnergyLevelModule::reset() {
		populate();
	}

	void EnergyLevelModule::update() {
		reset();
	}

	std::optional<Buffer> EnergyLevelModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (!source)
				return {};

			if (auto agent = std::dynamic_pointer_cast<Agent>(energyHaver); agent && source->getGID() == agent->getGID()) {
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			if (auto agent = std::dynamic_pointer_cast<Agent>(energyHaver))
				return Buffer{agent->getGID()};

		} else if (name == "UpdateEnergy") {

			auto *has_energy = std::any_cast<std::shared_ptr<HasEnergy>>(&data);
			assert(has_energy != nullptr);
			updateIf(*has_energy);

		}

		return {};
	}

	void EnergyLevelModule::updateIf(const std::shared_ptr<HasEnergy> &has_energy) {
		if (has_energy == energyHaver)
			update();
	}

	void EnergyLevelModule::populate() {
		if (!energyHaver)
			return;

		if (auto agent = std::dynamic_pointer_cast<Agent>(energyHaver))
			headerLabel.set_text(agent->getName());
		else
			headerLabel.set_text("???");

		EnergyAmount max{}, amount{};
		{
			auto lock = energyHaver->energyContainer->sharedLock();
			max = energyHaver->getEnergyCapacity();
			amount = energyHaver->getEnergy();
		}
		bar.set_fraction(amount < max? double(amount) / max : 1.);
	}
}
