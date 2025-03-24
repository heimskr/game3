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
		constexpr int WIDTH = 600;
		constexpr int HEIGHT = 350;
	}

	LoginDialog::LoginDialog(UIContext &ui, float selfScale):
		DraggableDialog(ui, selfScale, WIDTH, HEIGHT) {
			setTitle("Log In");
		}

	void LoginDialog::init() {
		DraggableDialog::init();

		auto vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 2, 0, Color{});

		auto grid = std::make_shared<Grid>(ui, selfScale);
		grid->setRowSpacing(5);

		auto make_label = [&](UString text) {
			auto label = std::make_shared<Label>(ui, selfScale, std::move(text));
			label->setVerticalAlignment(Alignment::Center);
			return label;
		};

		grid->attach(make_label("Username"), 0, 0);
		grid->attach(make_label("Name"), 1, 0);

		usernameInput = std::make_shared<TextInput>(ui, selfScale);
		usernameInput->setHorizontalExpand(true);
		usernameInput->onSubmit.connect([this](TextInput &, const UString &) { submit(true); });
		grid->attach(usernameInput, 0, 1);

		displayNameInput = std::make_shared<TextInput>(ui, selfScale);
		displayNameInput->setHorizontalExpand(true);
		displayNameInput->onSubmit.connect([this](TextInput &, const UString &) { submit(true); });
		grid->attach(displayNameInput, 1, 1);

		ui.window.settings.withShared([&](const ClientSettings &settings) {
			usernameInput->setText(settings.username);
		});

		vbox->append(std::move(grid));

		auto aligner = std::make_shared<Aligner>(ui, Orientation::Horizontal, Alignment::End);

		auto yes_icon = std::make_shared<Icon>(ui, selfScale);
		yes_icon->setIconTexture(cacheTexture("resources/gui/yes.png"));
		yes_icon->setFixedSize(8 * selfScale, 8 * selfScale);
		yes_icon->setOnClick(makeSubmit(true));

		aligner->setChild(std::move(yes_icon));
		vbox->append(std::move(aligner));
		vbox->insertAtEnd(shared_from_this());

		ui.focusWidget(usernameInput);
	}

	void LoginDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);
		firstChild->render(renderers, bodyRectangle);
	}

	void LoginDialog::submit(bool go) {
		if (go) {
			signalSubmit(usernameInput->getText(), displayNameInput->getText());
		} else {
			signalDismiss();
		}

		ui.removeDialog(getSelf());
	}

	std::function<bool(Widget &, int, int, int)> LoginDialog::makeSubmit(bool go) {
		return [this, go](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON)
				return false;
			submit(go);
			return true;
		};
	}
}
