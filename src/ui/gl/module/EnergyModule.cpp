#include "game/Agent.h"
#include "game/EnergyContainer.h"
#include "game/HasEnergy.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/ProgressBar.h"

namespace Game3 {
	EnergyModule::EnergyModule(UIContext &ui, float selfScale, const std::shared_ptr<ClientGame> &, const std::any &argument, bool show_header):
		EnergyModule(ui, selfScale, std::any_cast<AgentPtr>(argument), show_header) {}

	EnergyModule::EnergyModule(UIContext &ui, float selfScale, const AgentPtr &agent, bool show_header):
		Module(ui, selfScale), energyHaver(std::dynamic_pointer_cast<HasEnergy>(agent)), showHeader(show_header) {}

	void EnergyModule::init() {
		auto vbox = std::make_shared<Box>(ui, selfScale);
		vbox->setSeparatorThickness(0);

		if (showHeader) {
			auto label = std::make_shared<Label>(ui, selfScale);
			if (auto agent = std::dynamic_pointer_cast<Agent>(energyHaver)) {
				label->setText(agent->getName());
			} else {
				label->setText("???");
			}
			label->insertAtEnd(vbox);
		}

		auto hbox = std::make_shared<Box>(ui, selfScale, Orientation::Horizontal);
		hbox->setSeparatorThickness(0);

		auto label = std::make_shared<Label>(ui, selfScale);
		label->setText("Energy");
		label->setVerticalAlignment(Alignment::Center);
		label->insertAtEnd(hbox);

		bar = std::make_shared<ProgressBar>(ui, selfScale);
		bar->setFixedHeight(12 * selfScale);
		bar->setHorizontalExpand(true);
		bar->insertAtEnd(hbox);

		hbox->insertAtEnd(vbox);
		vbox->insertAtEnd(shared_from_this());
	}

	void EnergyModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);

		EnergyAmount max{}, amount{};
		{
			auto lock = energyHaver->energyContainer->sharedLock();
			max = energyHaver->getEnergyCapacity();
			amount = energyHaver->getEnergy();
		}

		bar->setProgress(amount < max && max != 0? static_cast<double>(amount) / max : 1.);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode EnergyModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void EnergyModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}
}
