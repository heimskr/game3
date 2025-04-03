#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "ui/gl/dialog/TitleDialog.h"
#include "ui/gl/widget/Aligner.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/IconButton.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	TitleDialog::TitleDialog(UIContext &ui, float selfScale):
		Dialog(ui, selfScale) {}

	void TitleDialog::init() {
		aligner = make<Aligner>(ui, Orientation::Horizontal, Alignment::End);

		hbox = make<Box>(ui, 1, Orientation::Horizontal);

		updateButton = make<IconButton>(ui, 1);
		updateButton->setIconTexture(cacheTexture("resources/gui/up.png"));
		updateButton->setTooltipText("Download update");
		updateButton->setOnClick([this](Widget &) {
			try {
				updater.update();
				ui.window.alert("Updated successfully.");
			} catch (const std::exception &error) {
				ui.window.error(std::format("Failed to update:\n{}", error.what()));
			}
		});

		updateButton->insertAtEnd(hbox);
		hbox->insertAtEnd(aligner);
		aligner->insertAtEnd(shared_from_this());
	}

	void TitleDialog::render(const RendererContext &renderers) {
		aligner->render(renderers, getPosition());
	}

	Rectangle TitleDialog::getPosition() const {
		Rectangle position = ui.window.inset(10);
		position.height = getScale() * 32;
		return position;
	}
}
