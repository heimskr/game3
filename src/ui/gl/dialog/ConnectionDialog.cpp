#include "util/Log.h"
#include "ui/gl/dialog/ConnectionDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/IntegerInput.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	ConnectionDialog::ConnectionDialog(UIContext &ui):
		Dialog(ui) {}

	void ConnectionDialog::init() {
		auto vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 2, 0, Color{});

		auto grid = std::make_shared<Grid>(ui, selfScale);
		grid->setRowSpacing(5);

		auto make_label = [&](UString text) {
			auto label = std::make_shared<Label>(ui, selfScale, std::move(text));
			label->setVerticalAlignment(Alignment::Center);
			return label;
		};

		grid->attach(make_label("Host"), 0, 0);
		grid->attach(make_label("Port"), 1, 0);

		hostInput = std::make_shared<TextInput>(ui, selfScale);
		hostInput->setText(ui.window.settings.hostname);
		hostInput->setHorizontalExpand(true);
		hostInput->onSubmit.connect([this](TextInput &, const UString &) { submit(); });
		grid->attach(hostInput, 0, 1);

		portInput = std::make_shared<IntegerInput>(ui, selfScale);
		portInput->setText(std::to_string(ui.window.settings.port));
		portInput->setHorizontalExpand(true);
		portInput->onSubmit.connect([this](TextInput &, const UString &) { submit(); });
		grid->attach(portInput, 1, 1);

		vbox->append(std::move(grid));
		vbox->insertAtEnd(shared_from_this());

		auto hbox = std::make_shared<Box>(ui, selfScale, Orientation::Horizontal, 2, 0, Color{});

		auto spacer = std::make_shared<Label>(ui, selfScale);
		spacer->setHorizontalExpand(true);

		auto local_button = std::make_shared<Button>(ui, selfScale);
		local_button->setText("Local Play");
		local_button->setOnClick([this](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON)
				return false;
			playLocally();
			return true;
		});

		auto connect_button = std::make_shared<Button>(ui, selfScale);
		connect_button->setText("Connect");
		connect_button->setOnClick([this](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON)
				return false;
			submit();
			return true;
		});

		hbox->append(std::move(spacer));
		hbox->append(std::move(local_button));
		hbox->append(std::move(connect_button));
		vbox->append(std::move(hbox));

		ui.focusWidget(hostInput);
	}

	void ConnectionDialog::render(const RendererContext &renderers) {
		Rectangle position = getPosition();

		{
			Rectangle frame_position = position;
			const int offset = 7 * getScale();
			frame_position.x -= offset;
			frame_position.y -= offset;
			frame_position.width += 2 * offset;
			frame_position.height += 2 * offset;
			auto saver = ui.scissorStack.pushAbsolute(frame_position, renderers);
			ui.drawFrame(renderers, selfScale, false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		firstChild->render(renderers, getPosition());
	}

	Rectangle ConnectionDialog::getPosition() const {
		// TODO: suspicious use of UI_SCALE
		constexpr int width = 600 * UI_SCALE / 8;
		constexpr int height = 284 * UI_SCALE / 8;
		return Rectangle((ui.getWidth() - width) / 2, (ui.getHeight() - height) / 2, width, height);
	}

	void ConnectionDialog::submit() {
		uint16_t port{};

		try {
			port = parseNumber<uint16_t>(portInput->getText().raw());
		} catch (const std::invalid_argument &) {
			ui.window.error("Invalid port number.");
			return;
		}

		std::string hostname = hostInput->getText().raw();

		ui.window.queue([hostname](Window &window) {
			if (window.isConnected()) {
				window.showLoginAndRegisterDialogs(hostname);
			}
		});

		try {
			ui.window.connect(hostname, port);
		} catch (const std::exception &err) {
			ui.window.error(err.what());
		}
	}

	void ConnectionDialog::playLocally() {
		ui.window.playLocally();
	}
}
