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

	void UIContext::render(float mouse_x, float mouse_y) {
		RendererContext context = canvas.getRendererContext();

		for (const std::shared_ptr<Dialog> &dialog: dialogs) {
			scissorStack = internalScissorStack;
			dialog->render(context);
		}

		if (draggedWidget && draggedWidgetActive) {
			scissorStack = internalScissorStack;
			const int factor = canvas.getFactor();
			const int width = canvas.getWidth() * factor * factor;
			const int height = canvas.getHeight() * factor * factor;
			GL::Viewport(0, 0, width, height);
			context.updateSize(width, height);
			renderingDraggedWidget = true;
			draggedWidget->render(*this, context, mouse_x * factor, mouse_y * factor + height / factor);
			renderingDraggedWidget = false;
		}
	}

	void UIContext::addDialog(std::shared_ptr<Dialog> dialog) {
		dialogs.emplace_back(std::move(dialog));
	}

	std::shared_ptr<ClientGame> UIContext::getGame() const {
		return canvas.game;
	}

	void UIContext::onResize(int x, int y) {
		internalScissorStack.setBase(Rectangle{0, 0, x, y});
	}

	bool UIContext::click(int x, int y) {
		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->click(x, y))
				return true;

		return false;
	}

	bool UIContext::dragStart(int x, int y) {
		dragOrigin.emplace(x, y);

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragStart(x, y))
				return true;

		return false;
	}

	bool UIContext::dragUpdate(int x, int y) {
		if (draggedWidget && dragOrigin != std::pair{x, y})
			draggedWidgetActive = true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragUpdate(x, y))
				return true;

		return false;
	}

	bool UIContext::dragEnd(int x, int y) {
		bool out = false;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs)) {
			if (dialog->dragEnd(x, y)) {
				out = true;
				break;
			}
		}

		draggedWidgetActive = false;
		setDraggedWidget(nullptr);

		return out;
	}

	void UIContext::setDraggedWidget(WidgetPtr new_dragged_widget) {
		draggedWidget = std::move(new_dragged_widget);
	}

	WidgetPtr UIContext::getDraggedWidget() const {
		return draggedWidget;
	}
}
