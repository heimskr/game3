#include "Log.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/MessageDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/IconButton.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	MessageDialog::MessageDialog(UIContext &ui, int width, int height, ButtonsType buttons_type):
		DraggableDialog(ui, width, height), buttonsType(buttons_type) {}

	void MessageDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);
		vbox->render(renderers, bodyRectangle);
	}

	void MessageDialog::init() {
		DraggableDialog::init();

		vbox = std::make_shared<Box>(ui, UI_SCALE, Orientation::Vertical, 2, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		buttonBox = std::make_shared<Box>(ui, UI_SCALE, Orientation::Horizontal, 2, 0, Color{});
		vbox->append(buttonBox);

		auto spacer = std::make_shared<Label>(ui, UI_SCALE);
		spacer->setHorizontalExpand(true);
		buttonBox->append(std::move(spacer));

		if (buttonsType == ButtonsType::Cancel || buttonsType == ButtonsType::CancelOkay) {
			auto cancel_button = std::make_shared<Button>(ui, UI_SCALE);
			cancel_button->setText("Cancel");
			cancel_button->setOnClick(makeSubmit(false));
			buttonBox->append(std::move(cancel_button));
		}

		if (buttonsType == ButtonsType::Okay || buttonsType == ButtonsType::CancelOkay) {
			auto okay_button = std::make_shared<Button>(ui, UI_SCALE);
			okay_button->setText("Okay");
			okay_button->setOnClick(makeSubmit(true));
			buttonBox->append(std::move(okay_button));
		}

		if (buttonsType == ButtonsType::No || buttonsType == ButtonsType::NoYes) {
			auto no_button = std::make_shared<Icon>(ui, UI_SCALE);
			no_button->setIconTexture(cacheTexture("resources/gui/no.png"));
			no_button->setFixedSize(8 * UI_SCALE, 8 * UI_SCALE);
			no_button->setOnClick(makeSubmit(false));
			buttonBox->append(std::move(no_button));
		}

		if (buttonsType == ButtonsType::Yes || buttonsType == ButtonsType::NoYes) {
			auto yes_button = std::make_shared<Icon>(ui, UI_SCALE);
			yes_button->setIconTexture(cacheTexture("resources/gui/yes.png"));
			yes_button->setFixedSize(8 * UI_SCALE, 8 * UI_SCALE);
			yes_button->setOnClick(makeSubmit(true));
			buttonBox->append(std::move(yes_button));
		}
	}

	bool MessageDialog::keyPressed(uint32_t key, Modifiers modifiers, bool) {
		if (key == GLFW_KEY_ESCAPE && modifiers.empty()) {
			submit(false);
			return true;
		}

		return false;
	}

	void MessageDialog::setChild(WidgetPtr new_child) {
		child = std::move(new_child);
		vbox->clearChildren();
		if (child != nullptr) {
			vbox->append(child);
		}
		vbox->append(buttonBox);
	}

	std::shared_ptr<MessageDialog> MessageDialog::create(UIContext &ui, UString text, ButtonsType buttons_type) {
		auto dialog = std::make_shared<MessageDialog>(ui, 750, 300, buttons_type);
		dialog->init();
		auto label = std::make_shared<Label>(ui, UI_SCALE);
		label->setExpand(true, true);
		label->setText(std::move(text));
		dialog->setChild(std::move(label));
		return dialog;
	}

	void MessageDialog::submit(bool response) {
		onSubmit(response);
		ui.removeDialog(getSelf());
	}

	std::function<bool(Widget &, int, int, int)> MessageDialog::makeSubmit(bool response) {
		return [this, response](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON)
				return false;
			submit(response);
			return true;
		};
	}
}
