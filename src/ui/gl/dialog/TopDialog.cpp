#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "ui/gl/dialog/TopDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	TopDialog::TopDialog(UIContext &ui):
		Dialog(ui) {}

	void TopDialog::init() {
		hbox = std::make_shared<Box>(ui, scale, Orientation::Horizontal, 0);

		abscond = make<Icon>(ui, scale);
		abscond->setIconTexture(cacheTexture("resources/gui/abscond.png"));
		abscond->setFixedSize(10 * scale);
		abscond->setTooltipText("Abscond");
		abscond->setOnClick([this](Widget &) {
			ui.window.disconnect();
		});
		hbox->append(abscond);

		hbox->insertAtEnd(shared_from_this());
	}

	void TopDialog::render(const RendererContext &renderers) {
		if (ui.window.isConnected()) {
			hbox->render(renderers, getPosition());
			return;
		}

		ui.getTooltip()->hide(*abscond);
	}

	Rectangle TopDialog::getPosition() const {
		float offset = scale;
		return Rectangle(offset, offset, ui.getWidth() - 2 * offset, 16 * scale);
	}

	void TopDialog::onBlur() {
		ui.getTooltip()->hide(*abscond);
	}
}
