#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/UIContext.h"
#include "ui/Canvas.h"
#include "util/Util.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	UIContext::UIContext(Canvas &canvas):
		canvas(canvas) {}

	void UIContext::render() {
		RendererContext context = canvas.getRendererContext();

		for (const std::unique_ptr<Dialog> &dialog: dialogs) {
			scissorStack = internalScissorStack;
			dialog->render(context);
		}
	}

	void UIContext::addDialog(std::unique_ptr<Dialog> &&dialog) {
		dialogs.emplace_back(std::move(dialog));
	}

	std::shared_ptr<ClientGame> UIContext::getGame() const {
		return canvas.game;
	}

	void UIContext::onResize(int x, int y) {
		internalScissorStack.setBase(Rectangle{0, 0, x, y});
	}

	bool UIContext::click(int x, int y) {
		for (const std::unique_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->click(x, y))
				return true;
		return false;
	}

	bool UIContext::dragStart(int x, int y) {
		for (const std::unique_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragStart(x, y))
				return true;
		return false;
	}
}
