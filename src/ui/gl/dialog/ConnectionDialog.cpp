#include "graphics/Texture.h"
#include "ui/gl/dialog/ConnectionDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/IntegerInput.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Spacer.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Log.h"
#include "util/Util.h"

namespace Game3 {
	ConnectionDialog::ConnectionDialog(UIContext &ui, float selfScale):
		Dialog(ui, selfScale) {}

	void ConnectionDialog::init() {
		auto vbox = make<Box>(ui, selfScale, Orientation::Vertical, 1, 0);

		auto grid = make<Grid>(ui, selfScale);
		grid->setRowSpacing(1);

		auto make_label = [&](UString text) {
			auto label = make<Label>(ui, selfScale, std::move(text));
			label->setVerticalAlignment(Alignment::Center);
			return label;
		};

		grid->attach(make_label("Host"), 0, 0);
		grid->attach(make_label("Port"), 1, 0);

		hostInput = make<TextInput>(ui, selfScale);
		hostInput->setText(ui.window.settings.hostname);
		hostInput->setHorizontalExpand(true);
		hostInput->onSubmit.connect([this](TextInput &, const UString &) { submit(); });
		grid->attach(hostInput, 0, 1);

		portInput = make<IntegerInput>(ui, selfScale);
		portInput->setText(std::to_string(ui.window.settings.port));
		portInput->setHorizontalExpand(true);
		portInput->onSubmit.connect([this](TextInput &, const UString &) { submit(); });
		grid->attach(portInput, 1, 1);

		vbox->append(std::move(grid));
		vbox->insertAtEnd(shared_from_this());

		auto hbox = make<Box>(ui, selfScale, Orientation::Horizontal, 2, 0);

		auto spacer = make<Spacer>(ui, Orientation::Horizontal);

		auto load_icon = make<Icon>(ui, selfScale, "resources/gui/folder.png");
		load_icon->setTooltipText("Load world");
		load_icon->setOnClick([this](Widget &) {
			loadWorld();
		});

		auto new_icon = make<Icon>(ui, selfScale, "resources/gui/plus.png");
		new_icon->setTooltipText("New world");
		new_icon->setOnClick([this](Widget &) {
			newWorld();
		});

		auto connect_button = make<Button>(ui, selfScale);
		connect_button->setText("Connect");
		connect_button->setOnClick([this](Widget &) {
			submit();
		});

		connect_button->setFixedHeight(connect_button->getMinimumPreferredHeight() / ui.scale);
		load_icon->setFixedSize(11);
		new_icon->setFixedSize(11);

		hbox->append(std::move(load_icon));
		hbox->append(std::move(new_icon));
		hbox->append(std::move(spacer));
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
			ui.drawFrame(renderers, getScale(), false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		firstChild->render(renderers, position);
	}

	Rectangle ConnectionDialog::getPosition() const {
		int width = 600 * ui.scale / 8;
		int height = 276 * ui.scale / 8;

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

	void ConnectionDialog::loadWorld() {
		ui.window.showWorldSelector();
	}

	void ConnectionDialog::newWorld() {
		ui.window.showWorldCreator();
	}
}
