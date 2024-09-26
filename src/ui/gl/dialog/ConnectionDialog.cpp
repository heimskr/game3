#include "ui/gl/dialog/ConnectionDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/IntegerInput.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	ConnectionDialog::ConnectionDialog(UIContext &ui):
		Dialog(ui) {}

	void ConnectionDialog::init() {
		auto vbox = std::make_shared<Box>(ui, UI_SCALE, Orientation::Vertical, 2, 0, Color{});

		auto grid = std::make_shared<Grid>(ui, UI_SCALE);
		grid->setRowSpacing(5);

		grid->attach(std::make_shared<Label>(ui, UI_SCALE, "Host"), 0, 0);
		grid->attach(std::make_shared<Label>(ui, UI_SCALE, "Port"), 1, 0);

		auto host_entry = std::make_shared<TextInput>(ui, UI_SCALE);
		host_entry->setHorizontalExpand(true);
		grid->attach(std::move(host_entry), 0, 1);

		auto port_entry = std::make_shared<IntegerInput>(ui, UI_SCALE);
		port_entry->setHorizontalExpand(true);
		grid->attach(std::move(port_entry), 1, 1);

		vbox->append(std::move(grid));
		vbox->insertAtEnd(shared_from_this());
	}

	void ConnectionDialog::render(const RendererContext &renderers) {
		Rectangle position = getPosition();

		{
			Rectangle frame_position = position;
			const int offset = 7 * scale;
			frame_position.x -= offset;
			frame_position.y -= offset;
			frame_position.width += 2 * offset;
			frame_position.height += 2 * offset;
			auto saver = ui.scissorStack.pushAbsolute(frame_position, renderers);
			ui.drawFrame(renderers, UI_SCALE, false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		firstChild->render(renderers, getPosition());
	}

	Rectangle ConnectionDialog::getPosition() const {
		constexpr int width = 400;
		constexpr int height = 400;
		return Rectangle((ui.getWidth() - width) / 2, (ui.getHeight() - height) / 2, width, height);
	}
}
