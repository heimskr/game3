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
#include "ui/MainWindow.h"

namespace Game3 {
	MutatorModule::MutatorModule(std::shared_ptr<ClientGame> game, const std::any &argument):
		MutatorModule(std::move(game), std::dynamic_pointer_cast<Mutator>(std::any_cast<AgentPtr>(argument))) {}

	MutatorModule::MutatorModule(std::shared_ptr<ClientGame> game, std::shared_ptr<Mutator> mutator):
		game(std::move(game)),
		mutator(std::move(mutator)),
		inventoryModule(std::make_shared<InventoryModule>(this->game, std::static_pointer_cast<ClientInventory>(this->mutator->getInventory(0)))),
		fluidsModule(std::make_shared<FluidsModule>(this->game, std::make_any<AgentPtr>(this->mutator), false)),
		geneInfoModule(std::make_shared<GeneInfoModule>(nullptr)) {}

	void MutatorModule::init(UIContext &ui) {
		inventoryModule->init(ui);
		fluidsModule->init(ui);
		geneInfoModule->init(ui);

		vbox = std::make_shared<Box>(scale, Orientation::Vertical, 5, 0, Color{});

		header = std::make_shared<Label>(scale);
		header->setText(ui, mutator->getName());
		header->setHorizontalAlignment(Alignment::Middle); // TODO: it's not aligning
		header->insertAtEnd(vbox);

		hbox = std::make_shared<Box>(scale, Orientation::Horizontal);

		inventoryModule->insertAtEnd(hbox);

		mutateButton = std::make_shared<Button>(scale);
		mutateButton->setText("Mutate");
		mutateButton->setFixedHeight(12 * scale);
		mutateButton->setVerticalAlignment(Alignment::Middle);
		mutateButton->setOnClick([this](Widget &, UIContext &, int button, int, int) {
			if (button != 1)
				return false;
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

	void MutatorModule::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(ui, renderers, x, y, width, height);
		vbox->render(ui, renderers, x, y, width, height);
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
		game->getPlayer()->sendMessage(mutator, "Mutate", buffer);
	}

	void MutatorModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		Module::setInventory(std::move(inventory));
		update();
	}

	std::optional<Buffer> MutatorModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == mutator->getGID()) {
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
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
}