#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/UIContext.h"
#include "ui/Canvas.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	UIContext::UIContext(Canvas &canvas):
		canvas(canvas) {}

	void UIContext::renderDialogs() {
		RendererContext context = canvas.getRendererContext();

		for (const std::unique_ptr<Dialog> &dialog: dialogs) {
			scissorStack = internalScissorStack;
			dialog->render(*this, context);
		}
	}

	void UIContext::addDialog(std::unique_ptr<Dialog> &&dialog) {
		dialogs.emplace_back(std::move(dialog));
	}
}
