#include "entity/Speaker.h"
#include "dialogue/Dialogue.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/widget/DialogueDisplay.h"
#include "ui/widget/Label.h"
#include "ui/UIContext.h"
#include "util/Log.h"

#include <GLFW/glfw3.h>


namespace Game3 {
	namespace {
		constexpr Color INACTIVE_OPTION_COLOR{"#4447"};
		constexpr Color ACTIVE_OPTION_COLOR{"#444f"};
		constexpr const char *INACTIVE_OPTION_TEXT = "- ";
		constexpr const char *ACTIVE_OPTION_TEXT = "> ";
	}

	DialogueRow::DialogueRow(UIContext &ui, float selfScale, DialogueOption option):
		Box(ui, selfScale, Orientation::Horizontal, 0, 0),
		option(std::move(option)) {}

	void DialogueRow::init() {
		indicator = make<Label>(ui, selfScale, INACTIVE_OPTION_TEXT, INACTIVE_OPTION_COLOR);
		text = make<Label>(ui, selfScale, option.display);
		append(indicator);
		append(text);
	}

	void DialogueRow::setActive(bool active) {
		if (active) {
			indicator->setText(ACTIVE_OPTION_TEXT);
			indicator->setTextColor(ACTIVE_OPTION_COLOR);
		} else {
			indicator->setText(INACTIVE_OPTION_TEXT);
			indicator->setTextColor(INACTIVE_OPTION_COLOR);
		}
	}

	DialogueDisplay::DialogueDisplay(UIContext &ui, float selfScale, DialogueGraphPtr graph):
		Box(ui, selfScale, Orientation::Vertical, 2, 0.5),
		graph(std::move(graph)) {}

	void DialogueDisplay::init() {
		WidgetPtr self = getSelf();

		if (auto speaker = graph->getSpeaker()) {
			faceTexture = speaker->getFaceTexture();
		}

		if (!faceTexture) {
			faceTexture = graph->getActiveNode()->faceOverride;
		}

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

	bool DialogueDisplay::keyPressed(uint32_t key, Modifiers, bool is_repeat) {
		if (is_repeat) {
			return false;
		}

		if (key == GLFW_KEY_ENTER) {
			activateOption();
			return true;
		}

		size_t option_count = optionBox->getChildCount();

		if (option_count <= 1) {
			return false;
		}

		if (key == GLFW_KEY_DOWN) {
			selectOption((selectedOptionIndex + 1) % option_count);
		} else if (key == GLFW_KEY_UP) {
			selectOption((selectedOptionIndex - 1) % option_count);
		} else {
			return false;
		}

		return true;
	}

	bool DialogueDisplay::getStillOpen() const {
		return graph && graph->getStillOpen();
	}

	TexturePtr DialogueDisplay::getFaceTexture() const {
		return faceTexture;
	}

	void DialogueDisplay::resetOptions(const std::shared_ptr<DialogueNode> &node) {
		selectedOptionIndex = 0;
		mainText->setText(node->getDisplay());
		optionBox->clearChildren();

		if (node->faceOverride) {
			faceTexture = node->faceOverride;
		}

		for (const DialogueOption &option: node->options) {
			auto row = make<DialogueRow>(ui, selfScale, option);
			row->setActive(optionBox->getChildCount() == selectedOptionIndex);
			optionBox->append(row);
		}

		maybeRemeasure(ui.getRenderers(0), -1, -1);
	}

	void DialogueDisplay::selectOption(size_t index) {
		assert(index < lastNode->options.size());

		if (index == selectedOptionIndex) {
			return;
		}

		WidgetPtr child = optionBox->getFirstChild();

		for (size_t i = 0; i < selectedOptionIndex; ++i) {
			child = child->getNextSibling();
		}

		std::dynamic_pointer_cast<DialogueRow>(child)->setActive(false);

		if (index < selectedOptionIndex) {
			for (size_t i = selectedOptionIndex; i > index; --i) {
				child = child->getPreviousSibling();
			}
		} else {
			for (size_t i = selectedOptionIndex; i < index; ++i) {
				child = child->getNextSibling();
			}
		}

		std::dynamic_pointer_cast<DialogueRow>(child)->setActive(true);

		selectedOptionIndex = index;
	}

	void DialogueDisplay::activateOption() {
		assert(lastNode != nullptr);
		graph->selectNode(lastNode->options.at(selectedOptionIndex).nodeTarget);
	}
}
