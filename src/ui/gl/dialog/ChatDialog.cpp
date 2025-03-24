#include "game/ClientGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "net/LocalClient.h"
#include "packet/SendChatMessagePacket.h"
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
		constexpr Color CHAT_SEPARATOR_COLOR{"#ffffffa0"};
		constexpr int CHAT_TOGGLER_SIZE = 64;
	}

	ChatDialog::ChatDialog(UIContext &ui):
		Dialog(ui) {}

	void ChatDialog::init() {
		scroller = std::make_shared<Scroller>(ui, scale, CHAT_SCROLLBAR_COLOR);
		messageBox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 2, 0, Color{});
		toggler = std::make_shared<Label>(ui, scale, "<<", CHAT_FOCUSED_TEXT_COLOR);

		vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 0, 0.5, CHAT_SEPARATOR_COLOR);

		toggler->setOnClick([this](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON) {
				return false;
			}
			toggle(false);
			return true;
		});

		messageInput = std::make_shared<TextInput>(ui, scale, Color{"#"}, Color{"#"}, CHAT_FOCUSED_TEXT_COLOR, CHAT_FOCUSED_TEXT_COLOR, 0);
		messageInput->onSubmit.connect([this](TextInput &input, const UString &text) {
			if (text.empty()) {
				return;
			}

			if (ClientGamePtr game = ui.getGame()) {
				if (text[0] == '/') {
					if (text.size() > 1) {
						try {
							game->runCommand(text.raw().substr(1));
						} catch (const std::exception &error) {
							game->handleChat(nullptr, std::format("Error: {}", error.what()));
						}
					}
				} else {
					game->getClient()->send(make<SendChatMessagePacket>(text.raw()));
				}

				input.clear();
			}
		});

		scroller->setChild(messageBox);
		scroller->setExpand(true, true);
		messageInput->setFixedHeight(8 * scale);
		vbox->append(scroller);
		vbox->append(messageInput);
		vbox->insertAtEnd(shared_from_this());
		toggler->insertAtEnd(shared_from_this());

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
				vbox->setSeparatorColor(CHAT_SEPARATOR_COLOR);
			} else {
				vbox->setSeparatorColor(Color{"#"});
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

	bool ChatDialog::scroll(float x_delta, float y_delta, int x, int y, Modifiers modifiers) {
		if (!isFocused()) {
			return false;
		}

		return Dialog::scroll(x_delta, y_delta, x, y, modifiers);
	}

	bool ChatDialog::keyPressed(uint32_t key, Modifiers, bool) {
		if (key == GLFW_KEY_ESCAPE && !isHidden && ui.getFocusedWidget() != messageInput) {
			setHidden(true);
			return true;
		}

		return false;
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
		auto label = std::make_shared<Label>(ui, scale, std::move(message), getTextColor());
		messageBox->append(std::move(label));
		// TODO: this is a temporary hack to fix the scrollbar. It's not otherwise informed that its child's height changed.
		scroller->lastChildHeight = -1;
		messageBox->maybeRemeasure(ui.getRenderers(), -1, -1);
	}

	void ChatDialog::toggle(bool affect_focus) {
		setHidden(!isHidden);

		if (affect_focus) {
			if (isHidden) {
				ui.unfocusWidget(messageInput);
				ui.unfocusDialog(getSelf());
			} else {
				ui.focusWidget(messageInput);
			}
		}
	}

	void ChatDialog::setHidden(bool hidden) {
		isHidden = hidden;

		if (isHidden) {
			toggler->setText(">>");
			ui.focusDialog(nullptr);
			ui.unfocusWidget(messageInput);
		} else {
			toggler->setText("<<");
			ui.focusDialog(getSelf());
		}
	}

	void ChatDialog::focusInput() {
		setHidden(false);
		ui.focusWidget(messageInput);
	}

	void ChatDialog::setSlash() {
		messageInput->setText("/");
	}

	Color ChatDialog::getTextColor() const {
		return isFocused()? CHAT_FOCUSED_TEXT_COLOR : CHAT_UNFOCUSED_TEXT_COLOR;
	}
}
