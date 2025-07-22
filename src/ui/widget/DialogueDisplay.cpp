#include "game/Dialogue.h"
#include "graphics/RendererContext.h"
#include "ui/widget/DialogueDisplay.h"
#include "ui/widget/Label.h"
#include "ui/UIContext.h"
#include "util/Log.h"

namespace Game3 {
	DialogueRow::DialogueRow(UIContext &ui, float selfScale, DialogueOption option):
		Box(ui, selfScale, Orientation::Horizontal, 0, 0),
		option(std::move(option)) {}

	void DialogueRow::init() {
		indicator = make<Label>(ui, selfScale, "  ");
		text = make<Label>(ui, selfScale, option.display);
		append(indicator);
		append(text);
	}

	void DialogueRow::setActive(bool active) {
		if (active) {
			indicator->setText("> ");
		} else {
			indicator->setText("  ");
		}
	}

	DialogueDisplay::DialogueDisplay(UIContext &ui, float selfScale, DialogueGraphPtr graph):
		Box(ui, selfScale, Orientation::Vertical, 2, 0.5),
		graph(std::move(graph)) {}

	void DialogueDisplay::init() {
		WidgetPtr self = getSelf();

		mainText = make<Label>(ui, selfScale, "");
		mainText->insertAtEnd(self);

		optionBox = make<Box>(ui, selfScale, Orientation::Vertical, 0, 0);
		optionBox->insertAtEnd(self);
	}

	void DialogueDisplay::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (auto current_node = graph->getActiveNode(); current_node != lastNode) {
			resetOptions(current_node);
			lastNode = std::move(current_node);
		}

		Box::render(renderers, x, y, width, height);
	}

	bool DialogueDisplay::getStillOpen() const {
		return graph && graph->getStillOpen();
	}

	void DialogueDisplay::resetOptions(const std::shared_ptr<DialogueNode> &node) {
		selectedOptionIndex = 0;
		mainText->setText(node->getDisplay());
		optionBox->clearChildren();

		for (const DialogueOption &option: node->options) {
			auto row = make<DialogueRow>(ui, selfScale, option);
			row->setActive(optionBox->getChildCount() == selectedOptionIndex);
			optionBox->append(row);
		}

		maybeRemeasure(ui.getRenderers(0), -1, -1);
	}
}
