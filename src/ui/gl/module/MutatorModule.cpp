#include "biology/Gene.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "tileentity/Mutator.h"
#include "ui/gl/module/FluidsModule.h"
#include "ui/gl/module/GeneInfoModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/MutatorModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/Constants.h"
#include "ui/GameUI.h"
#include "ui/Window.h"

namespace Game3 {
	MutatorModule::MutatorModule(UIContext &ui, float selfScale, const ClientGamePtr &game, const std::any &argument):
		MutatorModule(ui, selfScale, game, std::dynamic_pointer_cast<Mutator>(std::any_cast<AgentPtr>(argument))) {}

	MutatorModule::MutatorModule(UIContext &ui, float selfScale, const ClientGamePtr &game, std::shared_ptr<Mutator> mutator):
		Module(ui, selfScale, game),
		mutator(std::move(mutator)),
		inventoryModule(std::make_shared<InventoryModule>(ui, selfScale, std::static_pointer_cast<ClientInventory>(this->mutator->getInventory(0)))),
		fluidsModule(std::make_shared<FluidsModule>(ui, selfScale, game, std::make_any<AgentPtr>(this->mutator), false)),
		geneInfoModule(std::make_shared<GeneInfoModule>(ui, selfScale, nullptr)) {}

	void MutatorModule::init() {
		inventoryModule->init();
		fluidsModule->init();
		geneInfoModule->init();

		vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 5, 0, Color{});
		vbox->setHorizontalExpand(true);

		header = std::make_shared<Label>(ui, selfScale);
		header->setText(mutator->getName());
		header->setHorizontalAlignment(Alignment::Center);
		header->setHorizontalExpand(true);
		header->insertAtEnd(vbox);

		hbox = std::make_shared<Box>(ui, selfScale, Orientation::Horizontal);

		inventoryModule->insertAtEnd(hbox);

		mutateButton = std::make_shared<Button>(ui, selfScale);
		mutateButton->setText("Mutate");
		mutateButton->setFixedHeight(10 * selfScale);
		mutateButton->setVerticalAlignment(Alignment::Center);
		mutateButton->setOnClick([this](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON) {
				return false;
			}
			mutate();
			return true;
		});
		mutateButton->insertAtEnd(hbox);

		hbox->insertAtEnd(vbox);
		fluidsModule->insertAtEnd(vbox);
		geneInfoModule->insertAtEnd(vbox);

		vbox->insertAtEnd(shared_from_this());
	}

	void MutatorModule::reset() {
		inventoryModule->reset();
		fluidsModule->reset();
		geneInfoModule->reset();
	}

	void MutatorModule::update() {
		inventoryModule->update();
		fluidsModule->update();
		assert(mutator);
		geneInfoModule->update(std::shared_ptr<Gene>(mutator->getGene()));
	}

	void MutatorModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		vbox->render(renderers, x, y, width, height);
	}

	SizeRequestMode MutatorModule::getRequestMode() const {
		return vbox->getRequestMode();
	}

	void MutatorModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return vbox->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	void MutatorModule::mutate() {
		assert(mutator);
		std::any buffer = std::make_any<Buffer>(Side::Client);
		getGame()->getPlayer()->sendMessage(mutator, "Mutate", buffer);
	}

	void MutatorModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		Module::setInventory(std::move(inventory));
		update();
	}

	std::optional<Buffer> MutatorModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == mutator->getGID()) {
				getGame()->getWindow()->queue([](Window &window) {
					if (auto game_ui = window.getUI<GameUI>()) {
						game_ui->removeModule();
					}
				});
			}

		} else if (name == "GetAgentGID") {

			return Buffer{Side::Client, mutator->getGID()};

		} else if (std::optional<Buffer> buffer = inventoryModule->handleMessage(source, name, data)) {

			return buffer;

		} else if (std::optional<Buffer> buffer = fluidsModule->handleMessage(source, name, data)) {

			return buffer;

		}

		return std::nullopt;
	}

	std::shared_ptr<InventoryModule> MutatorModule::getPrimaryInventoryModule() {
		return inventoryModule;
	}
}
