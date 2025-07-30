#include "entity/Speaker.h"
#include "dialogue/Dialogue.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/widget/DialogueDisplay.h"
#include "ui/widget/Label.h"
#include "ui/Constants.h"
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

	DialogueDisplay::DialogueDisplay(UIContext &ui, float selfScale, DialogueNodePtr node):
		Scroller(ui, selfScale),
		node(std::move(node)) {}

	void DialogueDisplay::init() {
		mainBox = make<Box>(ui, selfScale, Orientation::Vertical, 2, 0.5);

		if (auto speaker = getGraph()->getSpeaker()) {
			faceTexture = speaker->getFaceTexture();
		}

		if (!faceTexture) {
			faceTexture = node->faceOverride;
		}

		mainText = make<Label>(ui, selfScale, "");
		mainText->insertAtEnd(mainBox);

		optionBox = make<Box>(ui, selfScale, Orientation::Vertical, 0, 0);
		optionBox->insertAtEnd(mainBox);

		setChild(mainBox);

		mainText->setText(node->getDisplay());

		if (node->faceOverride) {
			faceTexture = node->faceOverride;
		}

		for (const DialogueOption &option: node->options) {
			auto row = make<DialogueRow>(ui, selfScale, option);
			row->setActive(optionBox->getChildCount() == selectedOptionIndex);
			optionBox->append(row);
		}
	}

	void DialogueDisplay::render(const RendererContext &renderers, float, float, float, float) {
		Rectangle position = getPosition();

		{
			auto saver = ui.scissorStack.pushAbsolute(position, renderers);
			ui.drawFrame(renderers, getScale(), false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		Rectangle scroller_position = position.shrinkAll(7 * getScale());
		maybeRemeasure(renderers, scroller_position.width, scroller_position.height);
		Scroller::render(renderers, scroller_position.x, scroller_position.y, scroller_position.width, scroller_position.height);

		if (TexturePtr face_texture = getFaceTexture()) {
			float face_scale = getScale();
			position.x += position.width - face_scale * face_texture->width;
			position.y -= face_scale * face_texture->height;
			renderers.singleSprite.drawOnScreen(face_texture, RenderOptions::simple(position.x, position.y, face_scale, -1.0 / face_scale, -1.0 / face_scale));
		}
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

	TexturePtr DialogueDisplay::getFaceTexture() const {
		return faceTexture;
	}

	DialogueGraphPtr DialogueDisplay::getGraph() const {
		return node->getParent();
	}

	void DialogueDisplay::selectOption(size_t index) {
		assert(index < node->options.size());

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
		assert(node != nullptr);
		getGraph()->selectNode(node->options.at(selectedOptionIndex).nodeTarget);
	}

	Rectangle DialogueDisplay::getPosition() const {
		int ui_width = ui.getWidth();
		int ui_height = ui.getHeight();

		constexpr float x_leftover = 0.2;
		constexpr int height = 500;
		constexpr int y_gap = 32;

		return Rectangle(ui_width * x_leftover, ui_height - height - y_gap, ui_width * (1.0 - 2 * x_leftover), height);
	}
}
