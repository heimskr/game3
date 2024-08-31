#include "Options.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "types/Types.h"
#include "ui/gl/widget/HotbarWidget.h"
#include "ui/gl/widget/TooltipWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/UIContext.h"
#include "ui/Canvas.h"
#include "ui/Modifiers.h"
#include "util/Util.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
	constexpr float HOTBAR_SCALE = 6;
}

namespace Game3 {
	UIContext::UIContext(Canvas &canvas):
		canvas(canvas),
		hotbar(std::make_shared<HotbarWidget>(HOTBAR_SCALE)),
		tooltip(std::make_shared<TooltipWidget>(UI_SCALE)) {}

	void UIContext::render(float mouse_x, float mouse_y) {
		RendererContext context = canvas.getRendererContext();

		const int factor = canvas.getFactor();

		if (dialogs.empty()) {
			scissorStack = internalScissorStack;
			constexpr static float HOTBAR_BORDER = SLOT_PADDING * HOTBAR_SCALE / 3;
			constexpr static float width = (OUTER_SLOT_SIZE * HOTBAR_SIZE + SLOT_PADDING) * HOTBAR_SCALE + HOTBAR_BORDER * 2;
			constexpr static float height = (OUTER_SLOT_SIZE + SLOT_PADDING) * HOTBAR_SCALE + HOTBAR_BORDER * 2;
			hotbar->render(*this, context, (canvas.getWidth() * factor - width) / 2, canvas.getHeight() * factor - (OUTER_SLOT_SIZE * 2 - INNER_SLOT_SIZE / 2) * HOTBAR_SCALE, width, height);
		} else {
			for (const std::shared_ptr<Dialog> &dialog: dialogs) {
				scissorStack = internalScissorStack;
				dialog->render(context);
			}
		}

		tooltip->render(*this, context, mouse_x * factor, mouse_y * factor, -1, -1);

		if (draggedWidget && draggedWidgetActive) {
			scissorStack = internalScissorStack;
			const int width = canvas.getWidth() * factor * factor;
			const int height = canvas.getHeight() * factor * factor;
			GL::Viewport(0, 0, width, height);
			context.updateSize(width, height);
			renderingDraggedWidget = true;
			draggedWidget->render(*this, context, mouse_x * factor, mouse_y * factor + height / factor, -1, -1);
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

	bool UIContext::click(int button, int x, int y) {
		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->click(button, x, y))
				return true;

		return dialogs.empty() && hotbar->getLastRectangle().contains(x, y) && hotbar->click(*this, button, x, y);
	}

	bool UIContext::dragStart(int x, int y) {
		dragOrigin.emplace(x, y);

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragStart(x, y))
				return true;

		return dialogs.empty() && hotbar->getLastRectangle().contains(x, y) && hotbar->dragStart(*this, x, y);
	}

	bool UIContext::dragUpdate(int x, int y) {
		if (draggedWidget && dragOrigin != std::pair{x, y})
			draggedWidgetActive = true;

		for (const WidgetPtr &widget: extraDragUpdaters)
			if (widget->dragUpdate(*this, x, y))
				return true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragUpdate(x, y))
				return true;

		return dialogs.empty() && hotbar->getLastRectangle().contains(x, y) && hotbar->dragUpdate(*this, x, y);
	}

	bool UIContext::dragEnd(int x, int y) {
		if (auto pressed = getPressedWidget()) {
			return pressed->dragEnd(*this, x, y);
		}

		extraDragUpdaters.clear();

		bool out = false;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs)) {
			if (dialog->dragEnd(x, y)) {
				out = true;
				break;
			}
		}

		draggedWidgetActive = false;
		setDraggedWidget(nullptr);

		if (!out)
			return dialogs.empty() && hotbar->getLastRectangle().contains(x, y) && hotbar->dragEnd(*this, x, y);

		return true;
	}

	bool UIContext::scroll(float x_delta, float y_delta, int x, int y) {
		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->scroll(x_delta, y_delta, x, y))
				return true;

		return dialogs.empty() && hotbar->getLastRectangle().contains(x, y) && hotbar->scroll(*this, x_delta, y_delta, x, y);
	}

	bool UIContext::keyPressed(uint32_t character, Modifiers modifiers) {
		if (auto focused = getFocusedWidget())
			if (focused->keyPressed(*this, character, modifiers))
				return true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->keyPressed(character, modifiers))
				return true;

		return dialogs.empty() && hotbar->keyPressed(*this, character, modifiers);
	}

	void UIContext::setDraggedWidget(WidgetPtr new_dragged_widget) {
		draggedWidget = std::move(new_dragged_widget);
	}

	WidgetPtr UIContext::getDraggedWidget() const {
		return draggedWidget;
	}

	std::shared_ptr<ClientPlayer> UIContext::getPlayer() const {
		return canvas.game->getPlayer();
	}

	RendererContext UIContext::getRenderers() const {
		return canvas.getRendererContext();
	}

	void UIContext::focusWidget(std::weak_ptr<Widget> to_focus) {
		focusedWidget = std::move(to_focus);
	}

	WidgetPtr UIContext::getFocusedWidget() const {
		return focusedWidget.lock();
	}

	void UIContext::unfocus() {
		focusedWidget.reset();
	}

	void UIContext::setPressedWidget(std::weak_ptr<Widget> new_pressed) {
		pressedWidget = std::move(new_pressed);
	}

	WidgetPtr UIContext::getPressedWidget() const {
		return pressedWidget.lock();
	}

	void UIContext::unpress() {
		pressedWidget.reset();
	}

	std::pair<double, double> UIContext::getAbsoluteMouseCoordinates() const {
		const auto [x, y] = canvas.getMouseCoordinates();
		const auto factor = canvas.getFactor();
		return {x * factor, y * factor};
	}

	std::pair<double, double> UIContext::getRelativeMouseCoordinates() const {
		const auto [x, y] = getAbsoluteMouseCoordinates();
		const Rectangle top = scissorStack.getTop().rectangle;
		return {x - top.x, y - top.y};
	}

	bool UIContext::checkMouseRelative(const Rectangle &rectangle) const {
		const auto [x, y] = getRelativeMouseCoordinates();
		return rectangle.contains(static_cast<int>(x), static_cast<int>(y));
	}

	bool UIContext::checkMouseAbsolute(const Rectangle &rectangle) const {
		const auto [x, y] = getAbsoluteMouseCoordinates();
		return rectangle.contains(static_cast<int>(x), static_cast<int>(y));
	}

	std::shared_ptr<TooltipWidget> UIContext::getTooltip() const {
		return tooltip;
	}

	void UIContext::addDragUpdater(WidgetPtr widget) {
		extraDragUpdaters.emplace(std::move(widget));
	}
}
