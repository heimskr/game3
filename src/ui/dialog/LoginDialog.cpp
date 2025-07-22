#include "graphics/Texture.h"
#include "ui/dialog/LoginDialog.h"
#include "ui/widget/Aligner.h"
#include "ui/widget/Box.h"
#include "ui/widget/Icon.h"
#include "ui/widget/Grid.h"
#include "ui/widget/Label.h"
#include "ui/widget/TextInput.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	namespace {
		constexpr float WIDTH = 100;
		constexpr float HEIGHT = 35;
	}

	LoginDialog::LoginDialog(UIContext &ui, float selfScale):
		DraggableDialog(ui, selfScale, WIDTH, HEIGHT) {
			setTitle("Log In");
		}

	void LoginDialog::init() {
		DraggableDialog::init();

		auto vbox = make<Box>(ui, selfScale, Orientation::Vertical, 1, 0);

		auto grid = make<Grid>(ui, selfScale);
		grid->setRowSpacing(1);

		auto make_label = [&](UString text) {
			auto label = make<Label>(ui, selfScale, std::move(text));
			label->setVerticalAlignment(Alignment::Center);
			return label;
		};

		grid->attach(make_label("Username"), 0, 0);
		UString display_name_message = "Be sure to put something here if you're joining a new world";
		auto name_label = make_label("Name");
		name_label->setTooltipText(display_name_message);
		grid->attach(std::move(name_label), 1, 0);

		usernameInput = make<TextInput>(ui, selfScale);
		usernameInput->setHorizontalExpand(true);
		usernameInput->onSubmit.connect([this](TextInput &, const UString &) { submit(true); });
		grid->attach(usernameInput, 0, 1);

		auto hbox = make<Box>(ui, selfScale, Orientation::Horizontal, 1, 0);

		displayNameInput = make<TextInput>(ui, selfScale);
		displayNameInput->setTooltipText(std::move(display_name_message));
		displayNameInput->setHorizontalExpand(true);
		displayNameInput->onSubmit.connect([this](TextInput &, const UString &) { submit(true); });

		auto yes_icon = make<Icon>(ui, selfScale);
		yes_icon->setIconTexture(cacheTexture("resources/gui/yes.png"));
		yes_icon->setFixedSize(10 * selfScale);
		yes_icon->setOnClick(makeSubmit(true));

		hbox->append(displayNameInput);
		hbox->append(std::move(yes_icon));
		grid->attach(std::move(hbox), 1, 1);

		ui.window.settings.withShared([&](const ClientSettings &settings) {
			usernameInput->setText(settings.username);
		});

		grid->insertAtEnd(getSelf());

		ui.focusWidget(usernameInput);

		recenter();
	}

	void LoginDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);
		firstChild->render(renderers, bodyRectangle);
	}

	void LoginDialog::rescale(float new_scale) {
		position.width = WIDTH * new_scale;
		position.height = HEIGHT * new_scale;
		Dialog::rescale(new_scale);
	}

	void LoginDialog::onFocus() {
		DraggableDialog::onFocus();
		ui.focusWidget(usernameInput);
	}

	void LoginDialog::submit(bool go) {
		DialogPtr self = getSelf();

		if (go) {
			signalSubmit(usernameInput->getText(), displayNameInput->getText());
		} else {
			signalDismiss();
		}

		ui.removeDialog(self);
	}

	std::function<bool(Widget &, int, int, int)> LoginDialog::makeSubmit(bool go) {
		return [this, go](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON) {
				return false;
			}
			submit(go);
			return true;
		};
	}
}
