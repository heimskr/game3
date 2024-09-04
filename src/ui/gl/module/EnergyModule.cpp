#include "game/Agent.h"
#include "game/EnergyContainer.h"
#include "game/HasEnergy.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/ProgressBar.h"

namespace Game3 {
	EnergyModule::EnergyModule(std::shared_ptr<ClientGame> game, const std::any &argument, bool show_header):
		EnergyModule(std::move(game), std::any_cast<AgentPtr>(argument), show_header) {}

	EnergyModule::EnergyModule(std::shared_ptr<ClientGame>, const AgentPtr &agent, bool show_header):
		energyHaver(std::dynamic_pointer_cast<HasEnergy>(agent)), showHeader(show_header) {}

	void EnergyModule::init(UIContext &ui) {
		auto vbox = std::make_shared<Box>(scale);
		vbox->setSeparatorThickness(0);

		if (showHeader) {
			auto label = std::make_shared<Label>(scale);
			if (auto agent = std::dynamic_pointer_cast<Agent>(energyHaver))
				label->setText(ui, agent->getName());
			else
				label->setText(ui, "???");
			label->insertAtEnd(vbox);
		}

		auto hbox = std::make_shared<Box>(scale, Orientation::Horizontal);
		hbox->setSeparatorThickness(0);

		auto label = std::make_shared<Label>(scale);
		label->setText(ui, "Energy");
		label->setVerticalAlignment(Alignment::Middle);
		label->insertAtEnd(hbox);

		bar = std::make_shared<ProgressBar>(scale);
		bar->setFixedHeight(12 * scale);
		bar->setHorizontalExpand(true);
		bar->insertAtEnd(hbox);

		hbox->insertAtEnd(vbox);
		vbox->insertAtEnd(shared_from_this());
	}

	void EnergyModule::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(ui, renderers, x, y, width, height);

		EnergyAmount max{}, amount{};
		{
			auto lock = energyHaver->energyContainer->sharedLock();
			max = energyHaver->getEnergyCapacity();
			amount = energyHaver->getEnergy();
		}

		bar->setProgress(amount < max && max != 0? static_cast<double>(amount) / max : 1.);
		firstChild->render(ui, renderers, x, y, width, height);
	}

	SizeRequestMode EnergyModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void EnergyModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}
}
