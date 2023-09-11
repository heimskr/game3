#include "game/ClientGame.h"
#include "game/HasFluids.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/FluidLevelsModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	FluidLevelsModule::FluidLevelsModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	fluidHaver(std::dynamic_pointer_cast<HasFluids>(std::any_cast<AgentPtr>(argument))) {
		vbox.set_hexpand();
	}

	Gtk::Widget & FluidLevelsModule::getWidget() {
		return vbox;
	}

	void FluidLevelsModule::reset() {
		removeChildren(vbox);
		widgets.clear();
		populate();
	}

	void FluidLevelsModule::update() {
		reset();
	}

	std::optional<Buffer> FluidLevelsModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (!source)
				return {};

			if (auto agent = std::dynamic_pointer_cast<Agent>(fluidHaver); agent && source->getGID() == agent->getGID()) {
				// TODO!: nanogui
				// MainWindow &window = game->getWindow();
				// window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			if (auto agent = std::dynamic_pointer_cast<Agent>(fluidHaver))
				return Buffer{agent->getGID()};

		} else if (name == "UpdateFluids") {

			auto *has_fluids = std::any_cast<std::shared_ptr<HasFluids>>(&data);
			assert(has_fluids != nullptr);
			updateIf(*has_fluids);

		}

		return {};
	}

	void FluidLevelsModule::updateIf(const std::shared_ptr<HasFluids> &has_fluids) {
		if (has_fluids == fluidHaver)
			update();
	}

	void FluidLevelsModule::populate() {
		if (!fluidHaver)
			return;

		const auto &registry = game->registry<FluidRegistry>();
		auto &levels = fluidHaver->fluidContainer->levels;
		auto lock = levels.sharedLock();

		auto header = std::make_unique<Gtk::Label>("???");
		if (auto agent = std::dynamic_pointer_cast<Agent>(fluidHaver))
			header->set_text(agent->getName());
		header->set_margin(10);
		header->set_xalign(0.5);
		vbox.append(*header);
		widgets.push_back(std::move(header));

		for (const auto &[id, amount]: levels) {
			const FluidAmount max = fluidHaver->getMaxLevel(id);
			const double progress = amount < max? double(amount) / max : 1.;
			auto hbox  = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
			std::shared_ptr<Fluid> fluid = registry.maybe(id);
			assert(fluid);
			auto label = std::make_unique<Gtk::Label>(fluid->name);
			auto bar   = std::make_unique<Gtk::ProgressBar>();
			label->set_margin(5);
			bar->set_fraction(progress);
			bar->set_hexpand();
			bar->set_margin_end(5);
			bar->set_valign(Gtk::Align::CENTER);
			hbox->append(*label);
			hbox->append(*bar);
			vbox.append(*hbox);
			widgets.push_back(std::move(hbox));
			widgets.push_back(std::move(label));
			widgets.push_back(std::move(bar));
		}
	}
}
