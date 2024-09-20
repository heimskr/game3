#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Dialog::Dialog(UIContext &ui):
		ui(ui) {}

	void Dialog::onClose() {}

	bool Dialog::click(int, int x, int y) {
		return getPosition().contains(x, y);
	}

	bool Dialog::dragStart(int x, int y) {
		return getPosition().contains(x, y);
	}

	bool Dialog::dragUpdate(int x, int y) {
		return getPosition().contains(x, y);
	}

	bool Dialog::dragEnd(int x, int y) {
		return getPosition().contains(x, y);
	}

	bool Dialog::scroll(float, float, int x, int y) {
		return getPosition().contains(x, y);
	}

	bool Dialog::keyPressed(uint32_t, Modifiers) {
		return false;
	}
}
