#include "biology/Gene.h"
#include "ui/gl/module/GeneInfoModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"

namespace Game3 {
	GeneInfoModule::GeneInfoModule(GenePtr gene):
		gene(std::move(gene)) {}

	void GeneInfoModule::init(UIContext &ui) {
		vbox = std::make_shared<Box>(scale, Orientation::Vertical, 5, 0, Color{});
		vbox->insertAtEnd(shared_from_this());
	}

	void GeneInfoModule::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);
		vbox->render(ui, renderers, x, y, width, height);
	}

	SizeRequestMode GeneInfoModule::getRequestMode() const {
		return vbox->getRequestMode();
	}

	void GeneInfoModule::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		return vbox->measure(renderers, measure_orientation, for_width, for_height, minimum, natural);
	}

	void GeneInfoModule::reset() {
		vbox->clearChildren();

		if (!gene)
			return;

		for (const std::string &line: gene->describeLong()) {
			auto label = std::make_shared<Label>(scale);
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
