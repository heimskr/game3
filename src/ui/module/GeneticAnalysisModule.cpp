#include "biology/Gene.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/ContainmentOrb.h"
#include "net/Buffer.h"
#include "tileentity/Microscope.h"
#include "ui/gtk/Util.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/GeneticAnalysisModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	GeneticAnalysisModule::GeneticAnalysisModule(ClientGamePtr, const std::any &):
		GeneticAnalysisModule() {}

	GeneticAnalysisModule::GeneticAnalysisModule() {
		vbox.set_hexpand();
	}

	Gtk::Widget & GeneticAnalysisModule::getWidget() {
		return vbox;
	}

	void GeneticAnalysisModule::reset() {
		update(nullptr);
	}

	void GeneticAnalysisModule::update() {}

	void GeneticAnalysisModule::update(const ItemStackPtr &stack) {
		clearText();

		if (!stack)
			return;

		if (ContainmentOrb::validate(stack)) {
			ponderOrb(stack);
		} else if (stack->getID() == "base:item/gene") {
			analyzeGene(stack);
		} else if (stack->getID() == "base:item/genetic_template") {
			analyzeTemplate(stack);
		}
	}

	void GeneticAnalysisModule::ponderOrb(const ItemStackPtr &stack) {
		EntityPtr entity;

		try {
			entity = ContainmentOrb::makeEntity(stack);
		} catch (const std::exception &err) {
			ERROR("Couldn't create entity from containment orb in GeneticAnalysisModule: {}", err.what());
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
			vbox.append(*label);
			labels.push_back(std::move(label));
			return;
		}

		for (const std::string &description: descriptions) {
			addLabel(description);
		}
	}

	void GeneticAnalysisModule::analyzeGene(const ItemStackPtr &stack) {
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

	void GeneticAnalysisModule::analyzeTemplate(const ItemStackPtr &stack) {
		auto iter = stack->data.find("genes");
		if (iter == stack->data.end())
			return;

		bool first = true;

		for (const auto &[name, gene_json]: iter->items()) {
			std::unique_ptr<Gene> gene;
			try {
				gene = Gene::fromJSON(gene_json);
			} catch (const std::exception &) {
				continue;
			}

			if (first) {
				first = false;
			} else {
				Gtk::Separator &separator = separators.emplace_back(Gtk::Orientation::HORIZONTAL);
				separator.set_margin_top(10);
				separator.set_margin_bottom(5);
				vbox.append(separator);
			}

			for (const std::string &line: gene->describeLong()) {
				addLabel(line);
			}
		}
	}

	void GeneticAnalysisModule::addLabel(const std::string &text) {
		auto label = std::make_unique<Gtk::Label>(text);
		label->set_halign(Gtk::Align::START);
		label->set_margin_top(5);
		label->set_margin_start(5);
		vbox.append(*label);
		labels.push_back(std::move(label));
	}

	void GeneticAnalysisModule::clearText() {
		removeChildren(vbox);
		labels.clear();
		separators.clear();
	}
}
