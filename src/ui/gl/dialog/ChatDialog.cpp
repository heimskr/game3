#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/dialog/ChatDialog.h"
#include "ui/gl/widget/Aligner.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Hotbar.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	namespace {
		constexpr Color CHAT_SCROLLBAR_COLOR{"#ffffffa0"};
		constexpr Color CHAT_BACKGROUND_COLOR{"#000000a0"};
	}

	ChatDialog::ChatDialog(UIContext &ui):
		Dialog(ui) {}

	void ChatDialog::init() {
		auto scroller = std::make_shared<Scroller>(ui, scale, CHAT_SCROLLBAR_COLOR);
		auto aligner = std::make_shared<Aligner>(ui, Orientation::Vertical, Alignment::End);

		messageBox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 2, 0, Color{});

		// aligner->setChild(messageBox);
		// scroller->setChild(std::move(aligner));
		scroller->setChild(messageBox);
		scroller->insertAtEnd(shared_from_this());

		addMessage("Hello there.");
		addMessage("<Heimskr> test");

		onBlur();
	}

	void ChatDialog::render(const RendererContext &renderers) {
		const Rectangle position = getPosition();

		if (isFocused()) {
			renderers.rectangle.drawOnScreen(CHAT_BACKGROUND_COLOR, position);
		}

		firstChild->render(renderers, position);
	}

	Rectangle ChatDialog::getPosition() const {
		if (const std::optional<float> &last_y = ui.getHotbar()->getLastY()) {
			constexpr int height = 400;
			return Rectangle(0, *last_y - height - 64, ui.getWidth() / 2, height);
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
				label->setTextColor(Color{"#ffffff"});
			}
		}
	}

	void ChatDialog::onBlur() {
		for (WidgetPtr child = messageBox->getFirstChild(); child; child = child->getNextSibling()) {
			if (auto label = std::dynamic_pointer_cast<Label>(child)) {
				label->setTextColor(Color{"#000000a0"});
			}
		}
	}

	void ChatDialog::addMessage(UString message) {
		assert(messageBox != nullptr);
		auto label = std::make_shared<Label>(ui, scale, std::move(message));
		messageBox->append(std::move(label));
	}
}
