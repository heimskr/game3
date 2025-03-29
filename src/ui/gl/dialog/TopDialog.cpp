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
	TopDialog::TopDialog(UIContext &ui, float selfScale):
		Dialog(ui, selfScale) {}

	void TopDialog::init() {
		hbox = make<Box>(ui, selfScale, Orientation::Horizontal, 0);

		abscond = make<Icon>(ui, selfScale);
		abscond->setIconTexture(cacheTexture("resources/gui/abscond.png"));
		abscond->setFixedSize(10 * selfScale);
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
		float offset = selfScale;
		return Rectangle(0, 0, ui.getWidth(), 10 * getScale());
	}

	void TopDialog::onBlur() {
		ui.getTooltip()->hide(*abscond);
	}
}
