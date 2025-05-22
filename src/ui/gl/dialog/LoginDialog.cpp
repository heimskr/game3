#include "graphics/Texture.h"
#include "ui/gl/dialog/LoginDialog.h"
#include "ui/gl/widget/Aligner.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	namespace {
		constexpr float WIDTH = 75;
		constexpr float HEIGHT = 45;
	}

	LoginDialog::LoginDialog(UIContext &ui, float selfScale):
		DraggableDialog(ui, selfScale, WIDTH, HEIGHT) {
			setTitle("Log In");
		}

	void LoginDialog::init() {
		DraggableDialog::init();

		auto vbox = make<Box>(ui, selfScale, Orientation::Vertical, 2, 0, Color{});

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

		displayNameInput = make<TextInput>(ui, selfScale);
		displayNameInput->setTooltipText(std::move(display_name_message));
		displayNameInput->setHorizontalExpand(true);
		displayNameInput->onSubmit.connect([this](TextInput &, const UString &) { submit(true); });
		grid->attach(displayNameInput, 1, 1);

		ui.window.settings.withShared([&](const ClientSettings &settings) {
			usernameInput->setText(settings.username);
		});

		vbox->append(std::move(grid));

		auto aligner = make<Aligner>(ui, Orientation::Horizontal, Alignment::End);

		auto yes_icon = make<Icon>(ui, selfScale);
		yes_icon->setIconTexture(cacheTexture("resources/gui/yes.png"));
		yes_icon->setFixedSize(8 * selfScale, 8 * selfScale);
		yes_icon->setOnClick(makeSubmit(true));

		aligner->setChild(std::move(yes_icon));
		vbox->append(std::move(aligner));
		vbox->insertAtEnd(shared_from_this());

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
