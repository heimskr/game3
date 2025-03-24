#include "biology/Gene.h"
#include "ui/gl/module/GeneInfoModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"

namespace Game3 {
	GeneInfoModule::GeneInfoModule(UIContext &ui, float selfScale, GenePtr gene):
		Module(ui, selfScale), gene(std::move(gene)) {}

	void GeneInfoModule::init() {
		vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 0, 0, Color{});
		vbox->insertAtEnd(shared_from_this());
	}

	void GeneInfoModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		vbox->render(renderers, x, y, width, height);
	}

	SizeRequestMode GeneInfoModule::getRequestMode() const {
		return vbox->getRequestMode();
	}

	void GeneInfoModule::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		vbox->measure(renderers, measure_orientation, for_width, for_height, minimum, natural);
	}

	void GeneInfoModule::reset() {
		if (!vbox) {
			return;
		}

		vbox->clearChildren();

		if (!gene) {
			return;
		}

		for (const std::string &line: gene->describeLong()) {
			auto label = std::make_shared<Label>(ui, selfScale);
			label->setText(line);
			label->insertAtEnd(vbox);
		}
	}

	void GeneInfoModule::update() {
		reset();
	}

	void GeneInfoModule::update(std::shared_ptr<Gene> new_gene) {
		gene = std::move(new_gene);
		reset();
	}
}
