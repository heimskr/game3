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
		buttonBox = std::make_shared<Box>(ui, UI_SCALE, Orientation::Horizontal, 2, 0, Color{});

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

	bool MessageDialog::click(int button, int x, int y) {
		if (DraggableDialog::click(button, x, y))
			return true;

		return vbox->contains(x, y) && vbox->click(button, x, y);
	}

	bool MessageDialog::dragStart(int x, int y) {
		if (DraggableDialog::dragStart(x, y))
			return true;

		return vbox->contains(x, y) && vbox->dragStart(x, y);
	}

	bool MessageDialog::dragUpdate(int x, int y) {
		if (DraggableDialog::dragUpdate(x, y))
			return true;

		return vbox->contains(x, y) && vbox->dragUpdate(x, y);
	}

	bool MessageDialog::dragEnd(int x, int y) {
		if (DraggableDialog::dragEnd(x, y))
			return true;

		return vbox->contains(x, y) && vbox->dragEnd(x, y);
	}

	bool MessageDialog::scroll(float x_delta, float y_delta, int x, int y) {
		if (DraggableDialog::scroll(x_delta, y_delta, x, y))
			return true;

		return vbox->contains(x, y) && vbox->scroll(x_delta, y_delta, x, y);
	}

	bool MessageDialog::keyPressed(uint32_t character, Modifiers modifiers) {
		if (DraggableDialog::keyPressed(character, modifiers))
			return true;

		return vbox->keyPressed(character, modifiers);
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
		auto dialog = std::make_shared<MessageDialog>(ui, 600, 400, buttons_type);
		dialog->init();
		auto label = std::make_shared<Label>(ui, UI_SCALE);
		label->setExpand(true, true);
		label->setText(std::move(text));
		dialog->setChild(std::move(label));
		return dialog;
	}

	void MessageDialog::submit(bool response) {
		onSubmit(response);
		ui.removeDialog(shared_from_this());
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
