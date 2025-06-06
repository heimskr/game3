#include "biology/Gene.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/ContainmentOrb.h"
#include "net/Buffer.h"
#include "tileentity/Microscope.h"
#include "ui/gl/module/GeneticAnalysisModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/Window.h"

namespace Game3 {
	GeneticAnalysisModule::GeneticAnalysisModule(UIContext &ui, float selfScale, const ClientGamePtr &game, const std::any &):
		GeneticAnalysisModule(ui, selfScale, game) {}

	void GeneticAnalysisModule::init() {
		vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 0, 0, Color{});
		vbox->insertAtEnd(shared_from_this());
	}

	void GeneticAnalysisModule::reset() {
		update(nullptr);
	}

	void GeneticAnalysisModule::update() {}

	void GeneticAnalysisModule::update(const ItemStackPtr &stack) {
		clearText();

		if (!stack) {
			return;
		}

		if (ContainmentOrb::validate(stack)) {
			ponderOrb(stack);
		} else if (stack->getID() == "base:item/gene") {
			analyzeGene(stack);
		} else if (stack->getID() == "base:item/genetic_template") {
			analyzeTemplate(stack);
		}
	}

	void GeneticAnalysisModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(renderers, x, y, width, height);
		vbox->render(renderers, x, y, width, height);
	}

	SizeRequestMode GeneticAnalysisModule::getRequestMode() const {
		return vbox->getRequestMode();
	}

	void GeneticAnalysisModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return vbox->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	void GeneticAnalysisModule::ponderOrb(const ItemStackPtr &stack) {
		EntityPtr entity;

		try {
			entity = ContainmentOrb::makeEntity(stack);
		} catch (const std::exception &err) {
			ERR("Couldn't create entity from containment orb in GeneticAnalysisModule: {}", err.what());
			return;
		}

		if (entity->isPlayer()) {
			return;
		}

		auto living = std::dynamic_pointer_cast<LivingEntity>(entity);
		if (!living) {
			return;
		}

		std::vector<std::string> descriptions;

		living->iterateGenes([&](Gene &gene) {
			descriptions.push_back(gene.describeShort());
		});

		if (descriptions.empty()) {
			auto label = std::make_shared<Label>(ui, selfScale);
			label->setText("No genes found.");
			label->insertAtEnd(vbox);
			return;
		}

		for (const std::string &description: descriptions) {
			addLabel(description);
		}
	}

	void GeneticAnalysisModule::analyzeGene(const ItemStackPtr &stack) {
		const auto *object = stack->data.if_object();
		if (!object) {
			return;
		}

		const auto *gene_json = object->if_contains("gene");
		if (!gene_json) {
			return;
		}

		std::unique_ptr<Gene> gene = Gene::fromJSON(*gene_json);
		if (!gene) {
			return;
		}

		for (const std::string &line: gene->describeLong()) {
			addLabel(line);
		}
	}

	void GeneticAnalysisModule::analyzeTemplate(const ItemStackPtr &stack) {
		auto *object = stack->data.if_object();
		if (!object) {
			return;
		}

		auto *genes = object->if_contains("genes");
		if (!genes) {
			return;
		}

		auto *genes_object = genes->if_object();
		if (!genes_object) {
			return;
		}

		bool first = true;

		for (const auto &[name, gene_json]: *genes_object) {
			std::unique_ptr<Gene> gene;
			try {
				gene = Gene::fromJSON(gene_json);
			} catch (const std::exception &) {
				continue;
			}

			if (first) {
				first = false;
			} else {
				// Separator &separator = separators.emplace_back(Orientation::HORIZONTAL);
				// separator.set_margin_top(10);
				// separator.set_margin_bottom(5);
				// vbox.append(separator);
			}

			for (const std::string &line: gene->describeLong()) {
				addLabel(line);
			}
		}
	}

	void GeneticAnalysisModule::addLabel(const std::string &text) {
		auto label = std::make_shared<Label>(ui, selfScale);
		label->setText(text);
		label->insertAtEnd(vbox);
	}

	void GeneticAnalysisModule::clearText() {
		vbox->clearChildren();
	}
}
