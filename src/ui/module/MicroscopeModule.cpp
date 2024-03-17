#include "biology/Gene.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/ContainmentOrb.h"
#include "net/Buffer.h"
#include "tileentity/Microscope.h"
#include "ui/gtk/Util.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/MicroscopeModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	MicroscopeModule::MicroscopeModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		MicroscopeModule(game_, std::dynamic_pointer_cast<Microscope>(std::any_cast<AgentPtr>(argument))) {}

	MicroscopeModule::MicroscopeModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Microscope> microscope_):
	game(std::move(game_)),
	microscope(std::move(microscope_)),
	inventoryModule(std::make_shared<InventoryModule>(game, std::static_pointer_cast<ClientInventory>(microscope->getInventory(0)))) {
		vbox.set_hexpand();

		header.set_text(microscope->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);
		vbox.append(inventoryModule->getWidget());
		vbox.append(resultsBox);
	}

	Gtk::Widget & MicroscopeModule::getWidget() {
		return vbox;
	}

	void MicroscopeModule::reset() {
		inventoryModule->reset();
		updateResults();
	}

	void MicroscopeModule::update() {
		inventoryModule->update();
		updateResults();
	}

	void MicroscopeModule::updateResults() {
		if (!microscope)
			return;

		auto inventory = microscope->getInventory(0);
		auto lock = inventory->sharedLock();

		clearText();

		ItemStackPtr stack = (*inventory)[0];
		if (!stack)
			return;

		if (ContainmentOrb::validate(stack)) {
			analyzeOrb(stack);
		} else if (stack->getID() == "base:item/gene") {
			analyzeGene(stack);
		}
	}

	void MicroscopeModule::onResize(int width) {
		inventoryModule->onResize(width);
	}

	std::optional<Buffer> MicroscopeModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == microscope->getGID()) {
				inventoryModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{microscope->getGID()};

		} else {

			return inventoryModule->handleMessage(source, name, data);

		}

		return std::nullopt;
	}

	void MicroscopeModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		inventoryModule->setInventory(std::move(inventory));
		updateResults();
	}

	void MicroscopeModule::analyzeOrb(const ItemStackPtr &stack) {
		EntityPtr entity;

		try {
			entity = ContainmentOrb::makeEntity(stack);
		} catch (const std::exception &err) {
			ERROR("Couldn't create entity from containment orb in MicroscopeModule: {}", err.what());
			return;
		}

		if (entity->isPlayer())
			return;

		auto living = std::dynamic_pointer_cast<LivingEntity>(entity);
		if (!living)
			return;

		std::vector<std::string> descriptions;

		living->iterateGenes([&](Gene &gene) {
			descriptions.push_back(gene.describeShort());
		});

		if (descriptions.empty()) {
			auto label = std::make_unique<Gtk::Label>("No genes found.");
			resultsBox.append(*label);
			resultsLabels.push_back(std::move(label));
			return;
		}

		for (const std::string &description: descriptions) {
			addLabel(description);
		}
	}

	void MicroscopeModule::analyzeGene(const ItemStackPtr &stack) {
		auto iter = stack->data.find("gene");
		if (iter == stack->data.end())
			return;

		std::unique_ptr<Gene> gene;
		try {
			gene = Gene::fromJSON(*iter);
		} catch (const std::exception &) {
			return;
		}

		for (const std::string &line: gene->describeLong()) {
			addLabel(line);
		}
	}

	void MicroscopeModule::addLabel(const std::string &text) {
		auto label = std::make_unique<Gtk::Label>(text);
		label->set_halign(Gtk::Align::START);
		label->set_margin_top(5);
		label->set_margin_start(5);
		resultsBox.append(*label);
		resultsLabels.push_back(std::move(label));
	}

	void MicroscopeModule::clearText() {
		removeChildren(resultsBox);
		resultsLabels.clear();
	}
}
