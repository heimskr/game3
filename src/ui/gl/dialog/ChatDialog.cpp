#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/dialog/ChatDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Hotbar.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	namespace {
		constexpr Color CHAT_SCROLLBAR_COLOR{"#ffffffa0"};
		constexpr Color CHAT_BACKGROUND_COLOR{"#000000a0"};
		constexpr Color CHAT_FOCUSED_TEXT_COLOR{"#ffffff"};
		constexpr Color CHAT_UNFOCUSED_TEXT_COLOR{"#000000a0"};
		constexpr int CHAT_TOGGLER_SIZE = 64;
	}

	ChatDialog::ChatDialog(UIContext &ui):
		Dialog(ui) {}

	void ChatDialog::init() {
		scroller = std::make_shared<Scroller>(ui, scale, CHAT_SCROLLBAR_COLOR);
		messageBox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 2, 0, Color{});
		toggler = std::make_shared<Label>(ui, scale, "<<", CHAT_FOCUSED_TEXT_COLOR);

		vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 0, 0, Color{});

		toggler->setOnClick([this](Widget &, int button, int, int) {
			INFO("toggler onClick");
			if (button != LEFT_BUTTON)
				return false;
			toggle();
			return true;
		});

		messageInput = std::make_shared<TextInput>(ui, scale, Color{"#"}, Color{"#"}, CHAT_FOCUSED_TEXT_COLOR, CHAT_FOCUSED_TEXT_COLOR, 0);

		scroller->setChild(messageBox);
		scroller->setExpand(true, true);
		messageInput->setFixedHeight(8 * scale);
		vbox->append(scroller);
		vbox->append(messageInput);
		vbox->insertAtEnd(shared_from_this());
		toggler->insertAtEnd(shared_from_this());

		addMessage("Hello there.");
		addMessage("<Heimskr> test");

		onBlur();
	}

	void ChatDialog::render(const RendererContext &renderers) {
		Rectangle position = getPosition();
		position.width -= CHAT_TOGGLER_SIZE;

		Rectangle toggler_position = position;
		toggler_position.y = position.y + position.height - CHAT_TOGGLER_SIZE;
		toggler_position.width = CHAT_TOGGLER_SIZE;
		toggler_position.height = CHAT_TOGGLER_SIZE;

		if (!isHidden) {
			if (isFocused()) {
				renderers.rectangle.drawOnScreen(CHAT_BACKGROUND_COLOR, position);
			}

			vbox->render(renderers, position);
			toggler_position.x += position.width;
		} else {
			vbox->render(renderers, Rectangle{0, 0, -2, -2});
		}

		if (isFocused() || isHidden) {
			renderers.rectangle.drawOnScreen(CHAT_BACKGROUND_COLOR, toggler_position);
		}

		toggler->render(renderers, toggler_position);
	}

	Rectangle ChatDialog::getPosition() const {
		if (const std::optional<float> &last_y = ui.getHotbar()->getLastY()) {
			constexpr int height = 400;
			Rectangle out(0, *last_y - height - 64, isHidden? CHAT_TOGGLER_SIZE : ui.getWidth() / 2, height);
			if (isHidden) {
				out.y += out.height - CHAT_TOGGLER_SIZE;
				out.height = CHAT_TOGGLER_SIZE;
			}
			return out;
		}

		return Rectangle(0, 0, -1, -1);
	}

	bool ChatDialog::click(int button, int x, int y) {
		return Dialog::click(button, x, y);
	}

	bool ChatDialog::scroll(float x_delta, float y_delta, int x, int y) {
		if (!isFocused()) {
			return false;
		}

		return Dialog::scroll(x_delta, y_delta, x, y);
	}

	void ChatDialog::onFocus() {
		for (WidgetPtr child = messageBox->getFirstChild(); child; child = child->getNextSibling()) {
			if (auto label = std::dynamic_pointer_cast<Label>(child)) {
				label->setTextColor(CHAT_FOCUSED_TEXT_COLOR);
			}
		}
	}

	void ChatDialog::onBlur() {
		for (WidgetPtr child = messageBox->getFirstChild(); child; child = child->getNextSibling()) {
			if (auto label = std::dynamic_pointer_cast<Label>(child)) {
				label->setTextColor(CHAT_UNFOCUSED_TEXT_COLOR);
			}
		}
	}

	void ChatDialog::addMessage(UString message) {
		assert(messageBox != nullptr);
		auto label = std::make_shared<Label>(ui, scale, std::move(message));
		messageBox->append(std::move(label));
	}

	void ChatDialog::toggle() {
		isHidden = !isHidden;

		if (isHidden) {
			toggler->setText(">>");
			ui.focusDialog(nullptr);
			ui.unfocusWidget(messageInput);
		} else {
			toggler->setText("<<");
			ui.focusDialog(getSelf());
		}
	}
}