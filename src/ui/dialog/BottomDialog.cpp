#include "graphics/RendererContext.h"
#include "ui/dialog/BottomDialog.h"
#include "ui/widget/Hotbar.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	BottomDialog::BottomDialog(UIContext &ui, float selfScale):
		Dialog(ui, selfScale) {}

	void BottomDialog::init() {
		hotbar = make<Hotbar>(ui, selfScale * 0.75);
		hotbar->setName("Hotbar");
		hotbar->insertAtEnd(getSelf());
	}

	void BottomDialog::render(const RendererContext &renderers) {
		if (!shouldShowHotbar()) {
			return;
		}

		int ui_width = ui.getWidth();
		int ui_height = ui.getHeight();
		float dummy{};
		float natural_width{};
		float natural_height{};
		hotbar->measure(renderers, Orientation::Horizontal, ui_width, ui_height, dummy, natural_width);
		hotbar->measure(renderers, Orientation::Vertical, ui_width, ui_height, dummy, natural_height);
		hotbar->render(renderers, (ui_width - natural_width) / 2, ui_height - natural_height - hotbar->getScale(), natural_width, natural_height);
	}

	Rectangle BottomDialog::getPosition() const {
		return Rectangle(0, 0, ui.getWidth(), ui.getHeight());
	}

	std::shared_ptr<Hotbar> BottomDialog::getHotbar() const {
		return hotbar;
	}

	bool BottomDialog::shouldShowHotbar() const {
		return ui.window.isConnected() && std::ranges::none_of(ui.getDialogs(), +[](const DialogPtr &dialog) { return dialog->hidesHotbar(); });
	}
}
