#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "minigame/Minigame.h"
#include "ui/gl/dialog/MinigameDialog.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	MinigameDialog::MinigameDialog(UIContext &ui, std::shared_ptr<Minigame> minigame, int width, int height):
		DraggableDialog(ui, BaseDraggableDialog::getEffectiveWidth(width, UI_SCALE), BaseDraggableDialog::getEffectiveHeight(height, UI_SCALE)),
		width(width),
		height(height),
		minigame(std::move(minigame)) {
			setTitle(this->minigame->getName());
		}

	void MinigameDialog::init() {
		DraggableDialog::init();

		lastTime = std::chrono::system_clock::now();
		minigame->setSize(width, height);
		minigame->reset();
		minigame->insertAtEnd(shared_from_this());
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
}
