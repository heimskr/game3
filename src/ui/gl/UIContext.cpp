#include "Options.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "types/Types.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/ContextMenu.h"
#include "ui/gl/widget/Hotbar.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "ui/Modifiers.h"
#include "util/Util.h"

namespace Game3 {
	UIContext::UIContext(Window &window):
		window(window),
		hotbar(std::make_shared<Hotbar>(*this, HOTBAR_SCALE)),
		tooltip(std::make_shared<Tooltip>(*this, UI_SCALE)) {
			hotbar->init();
			tooltip->init();
		}

	void UIContext::render(float mouse_x, float mouse_y) {
		RendererContext context = window.getRendererContext();

		const int factor = window.getFactor();

		if (dialogs.empty()) {
			scissorStack = internalScissorStack;
			constexpr static float width = (OUTER_SLOT_SIZE * HOTBAR_SIZE + SLOT_PADDING) * HOTBAR_SCALE + HOTBAR_BORDER * 2;
			hotbar->render(context, (window.getWidth() * factor - width) / 2, window.getHeight() * factor - (OUTER_SLOT_SIZE * 2 - INNER_SLOT_SIZE / 2) * HOTBAR_SCALE, -1, -1);
		} else {
			for (const std::shared_ptr<Dialog> &dialog: dialogs) {
				scissorStack = internalScissorStack;
				dialog->render(context);
			}
		}

		if (autocompleteDropdown)
			autocompleteDropdown->render(context, -1, -1, -1, -1); // The dropdown is responsible for knowing where to render.

		if (contextMenu)
			contextMenu->render(context, -1, -1, -1, -1); // Same here.

		if (tooltip)
			tooltip->render(context, mouse_x * factor, mouse_y * factor, -1, -1);

		if (draggedWidget && draggedWidgetActive) {
			scissorStack = internalScissorStack;
			const int width = window.getWidth() * factor * factor;
			const int height = window.getHeight() * factor * factor;
			GL::Viewport(0, 0, width, height);
			context.updateSize(width, height);
			renderingDraggedWidget = true;
			draggedWidget->render(context, mouse_x * factor, mouse_y * factor + height / factor, -1, -1);
			renderingDraggedWidget = false;
		}
	}

	void UIContext::addDialog(std::shared_ptr<Dialog> dialog) {
		dialogs.emplace_back(std::move(dialog));
	}

	std::shared_ptr<ClientGame> UIContext::getGame() const {
		return window.game;
	}

	void UIContext::onResize(int x, int y) {
		internalScissorStack.setBase(Rectangle{0, 0, x, y});
	}

	void UIContext::reset() {
		draggedWidget = nullptr;
		draggedWidgetActive = false;
		for (const DialogPtr &dialog: reverse(dialogs))
			dialog->onClose();
		dialogs.clear();
		hotbar->reset();
	}

	bool UIContext::click(int button, int x, int y) {
		if (contextMenu && contextMenu->click(button, x, y))
			return true;

		if (autocompleteDropdown && autocompleteDropdown->click(button, x, y))
			return true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->click(button, x, y))
				return true;

		return dialogs.empty() && hotbar->contains(x, y) && hotbar->click(button, x, y);
	}

	bool UIContext::dragStart(int x, int y) {
		if (contextMenu && contextMenu->dragStart(x, y))
			return true;

		if (autocompleteDropdown && autocompleteDropdown->dragStart(x, y))
			return true;

		unfocus();
		dragOrigin.emplace(x, y);

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragStart(x, y))
				return true;

		return dialogs.empty() && hotbar->contains(x, y) && hotbar->dragStart(x, y);
	}

	bool UIContext::dragUpdate(int x, int y) {
		if (contextMenu && contextMenu->dragUpdate(x, y))
			return true;

		if (autocompleteDropdown && autocompleteDropdown->dragUpdate(x, y))
			return true;

		if (draggedWidget && dragOrigin != std::pair{x, y})
			draggedWidgetActive = true;

		for (const WidgetPtr &widget: extraDragUpdaters)
			if (widget->dragUpdate(x, y))
				return true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->dragUpdate(x, y))
				return true;

		return dialogs.empty() && hotbar->contains(x, y) && hotbar->dragUpdate(x, y);
	}

	bool UIContext::dragEnd(int x, int y) {
		if (contextMenu && contextMenu->dragEnd(x, y))
			return true;

		if (autocompleteDropdown && autocompleteDropdown->dragEnd(x, y))
			return true;

		if (auto pressed = getPressedWidget())
			return pressed->dragEnd(x, y);

		for (const WidgetPtr &widget: extraDragUpdaters) {
			widget->dragOrigin.reset();
			widget->dragEnd(x, y);
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
			return dialogs.empty() && hotbar->contains(x, y) && hotbar->dragEnd(x, y);

		return true;
	}

	bool UIContext::scroll(float x_delta, float y_delta, int x, int y) {
		if (contextMenu && contextMenu->scroll(x_delta, y_delta, x, y))
			return true;

		if (autocompleteDropdown && autocompleteDropdown->scroll(x_delta, y_delta, x, y))
			return true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->scroll(x_delta, y_delta, x, y))
				return true;

		return dialogs.empty() && hotbar->contains(x, y) && hotbar->scroll(x_delta, y_delta, x, y);
	}

	bool UIContext::keyPressed(uint32_t character, Modifiers modifiers) {
		if (auto focused = getFocusedWidget())
			if (focused->keyPressed(character, modifiers))
				return true;

		if (auto context_menu = getContextMenu())
			if (context_menu->keyPressed(character, modifiers))
				return true;

		for (const std::shared_ptr<Dialog> &dialog: reverse(dialogs))
			if (dialog->keyPressed(character, modifiers))
				return true;

		return dialogs.empty() && hotbar->keyPressed(character, modifiers);
	}

	void UIContext::setDraggedWidget(WidgetPtr new_dragged_widget) {
		draggedWidget = std::move(new_dragged_widget);
	}

	WidgetPtr UIContext::getDraggedWidget() const {
		return draggedWidget;
	}

	std::shared_ptr<ClientPlayer> UIContext::getPlayer() const {
		return window.game->getPlayer();
	}

	RendererContext UIContext::getRenderers() const {
		return window.getRendererContext();
	}

	void UIContext::focusWidget(const WidgetPtr &to_focus) {
		auto locked = focusedWidget.lock();

		if (to_focus == locked)
			return;

		if (locked)
			locked->onBlur();

		focusedWidget = to_focus;

		if (to_focus)
			to_focus->onFocus();
	}

	WidgetPtr UIContext::getFocusedWidget() const {
		return focusedWidget.lock();
	}

	void UIContext::unfocus() {
		if (auto widget = focusedWidget.lock())
			widget->onBlur();
		focusedWidget.reset();
	}

	void UIContext::setPressedWidget(const WidgetPtr &new_pressed) {
		pressedWidget = new_pressed;
	}

	WidgetPtr UIContext::getPressedWidget() const {
		return pressedWidget.lock();
	}

	void UIContext::setAutocompleteDropdown(std::shared_ptr<AutocompleteDropdown> new_dropdown) {
		autocompleteDropdown = std::move(new_dropdown);
	}

	std::shared_ptr<AutocompleteDropdown> UIContext::getAutocompleteDropdown() const {
		return autocompleteDropdown;
	}

	void UIContext::unpress() {
		pressedWidget.reset();
	}

	std::pair<double, double> UIContext::getAbsoluteMouseCoordinates() const {
		const auto [x, y] = window.getMouseCoordinates();
		const auto factor = window.getFactor();
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

	std::shared_ptr<Tooltip> UIContext::getTooltip() const {
		return tooltip;
	}

	void UIContext::addDragUpdater(WidgetPtr widget) {
		extraDragUpdaters.emplace(std::move(widget));
	}

	bool UIContext::anyDragUpdaters() const {
		return !extraDragUpdaters.empty();
	}

	void UIContext::setContextMenu(std::shared_ptr<ContextMenu> new_context_menu) {
		contextMenu = std::move(new_context_menu);
	}

	std::shared_ptr<ContextMenu> UIContext::getContextMenu() const {
		return contextMenu;
	}

	void UIContext::drawFrame(const RendererContext &renderers, double scale, bool alpha, const std::array<std::string_view, 8> &pieces, const Color &interior) {
		const Rectangle rectangle = scissorStack.getTop().rectangle;
		SingleSpriteRenderer &single = renderers.singleSprite;

		TexturePtr top_left     = cacheTexture(pieces[0], alpha);
		TexturePtr top          = cacheTexture(pieces[1], alpha);
		TexturePtr top_right    = cacheTexture(pieces[2], alpha);
		TexturePtr right        = cacheTexture(pieces[3], alpha);
		TexturePtr bottom_right = cacheTexture(pieces[4], alpha);
		TexturePtr bottom       = cacheTexture(pieces[5], alpha);
		TexturePtr bottom_left  = cacheTexture(pieces[6], alpha);
		TexturePtr left         = cacheTexture(pieces[7], alpha);

		single.drawOnScreen(top_left, RenderOptions{
			.x = 0,
			.y = 0,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		single.drawOnScreen(top, RenderOptions{
			.x = top_left->width * scale,
			.y = 0,
			.sizeX = rectangle.width - (top_left->width + top_right->width) * scale,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(top_right, RenderOptions{
			.x = rectangle.width - top_right->width * scale,
			.y = 0,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		single.drawOnScreen(left, RenderOptions{
			.x = 0,
			.y = top_left->height * scale,
			.sizeX = -1,
			.sizeY = rectangle.height - (top_left->height + bottom_left->height) * scale,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(right, RenderOptions{
			.x = rectangle.width - top_right->width * scale,
			.y = top_right->height * scale,
			.sizeX = -1,
			.sizeY = rectangle.height - (top_right->height + bottom_right->height) * scale,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(bottom_left, RenderOptions{
			.x = 0,
			.y = rectangle.height - bottom_left->height * scale,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		single.drawOnScreen(bottom, RenderOptions{
			.x = bottom_left->width * scale,
			.y = rectangle.height - bottom->height * scale,
			.sizeX = rectangle.width - (bottom_left->width + bottom_right->width) * scale,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(bottom_right, RenderOptions{
			.x = rectangle.width - bottom_right->width * scale,
			.y = rectangle.height - bottom_right->height * scale,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		if (interior.alpha > 0) {
			renderers.rectangle.drawOnScreen(interior, left->width * scale, top->height * scale, rectangle.width - (left->width + right->width) * scale, rectangle.height - (top->height + bottom->height) * scale);
		}
	}
}
