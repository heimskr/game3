#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "minigame/Minigame.h"
#include "ui/gl/dialog/MinigameDialog.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	MinigameDialog::MinigameDialog(UIContext &ui, float selfScale, std::shared_ptr<Minigame> minigame, int width, int height):
		DraggableDialog(ui, selfScale, BaseDraggableDialog::getEffectiveWidth(width * ui.scale / 8, selfScale * ui.scale), BaseDraggableDialog::getEffectiveHeight(height * ui.scale / 8, selfScale * ui.scale)),
		width(width),
		height(height),
		minigame(std::move(minigame)) {
			assert(this->minigame != nullptr);
			setTitle(this->minigame->getGameName());
		}

	void MinigameDialog::init() {
		DraggableDialog::init();

		lastTime = std::chrono::system_clock::now();
		minigame->setSize(width * selfScale * ui.scale / 8, height * selfScale * ui.scale / 8);
		minigame->reset();
		minigame->insertAtEnd(shared_from_this());
	}

	void MinigameDialog::rescale(float scale) {
		minigame->setSize(width * selfScale * scale / 8, height * selfScale * scale / 8);
	}

	void MinigameDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);

		if (!minigame) {
			return;
		}

		auto now = std::chrono::system_clock::now();
		double delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastTime).count() / 1e9;
		lastTime = now;
		minigame->tick(delta);
		minigame->render(renderers, getInnerRectangle());
	}

	void MinigameDialog::onClose() {
		Dialog::onClose();
		minigame->onClose();
	}

	void MinigameDialog::onFocus() {
		minigame->onFocus();
	}

	void MinigameDialog::onBlur() {
		minigame->onBlur();
	}
}
