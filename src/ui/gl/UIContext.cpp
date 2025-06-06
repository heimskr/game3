#include "Options.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "types/Types.h"
#include "ui/gl/dialog/Dialog.h"
#include "ui/gl/dialog/TopDialog.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/ContextMenu.h"
#include "ui/gl/widget/Hotbar.h"
#include "ui/gl/widget/StatusEffectsDisplay.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Modifiers.h"
#include "ui/UI.h"
#include "ui/Window.h"
#include "util/Defer.h"
#include "util/Reverse.h"

namespace Game3 {
	UIContext::UIContext(Window &window):
		UIContext(window, UI_SCALE) {}

	UIContext::UIContext(Window &window, float scale):
		window(window),
		scale(scale),
		hotbar(std::make_shared<Hotbar>(*this, 0.75)),
		tooltip(std::make_shared<Tooltip>(*this, 1)) {
			hotbar->setName("Hotbar");
			hotbar->init();
			tooltip->init();
		}

	void UIContext::render(float mouse_x, float mouse_y, float delta) {
		RendererContext context = window.getRendererContext(delta);

		const int x_factor = window.getXFactor();
		const int y_factor = window.getYFactor();

		if (currentUI) {
			currentUI->render(context);
		}

		if (ClientGamePtr game = getGame(); game != nullptr && game->getActiveRealm() != nullptr) {
			if (std::ranges::none_of(dialogs, +[](const DialogPtr &dialog) { return dialog->hidesHotbar(); })) {
				scissorStack = internalScissorStack;
				float dummy{};
				float natural_width{};
				float natural_height{};
				hotbar->measure(context, Orientation::Horizontal, window.getWidth(), window.getHeight(), dummy, natural_width);
				hotbar->measure(context, Orientation::Vertical, window.getWidth(), window.getHeight(), dummy, natural_height);
				hotbar->render(context, (window.getWidth() - natural_width) / 2, window.getHeight() - natural_height - hotbar->getScale(), natural_width, natural_height);
			}

			StatusEffectsDisplay(*this, 1).render(context, internalScissorStack.getTop().rectangle);
		}

		for (const DialogPtr &dialog: dialogs) {
			if (dialog != currentUI) {
				scissorStack = internalScissorStack;
				dialog->render(context);
			}
		}

		if (autocompleteDropdown) {
			// The dropdown is responsible for knowing where to render.
			autocompleteDropdown->render(context, -1, -1, -1, -1);
		}

		if (contextMenu) {
			// Same here.
			contextMenu->render(context, -1, -1, -1, -1);
		}

		if (tooltip) {
			// The tooltip knows its size but needs to be told its position.
			tooltip->render(context, mouse_x * x_factor, mouse_y * y_factor, -1, -1);
		}

		if (draggedWidget && draggedWidgetActive) {
			scissorStack = internalScissorStack;
			const int width = window.getWidth();
			const int height = window.getHeight();
			GL::Viewport(0, 0, width, height);
			context.updateSize(width, height);
			renderingDraggedWidget = true;
			draggedWidget->render(context, mouse_x * x_factor, mouse_y * y_factor, -1, -1);
			renderingDraggedWidget = false;
		}
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
		for (const DialogPtr &dialog: reverse(dialogs)) {
			dialog->onClose();
		}
		dialogs.clear();
		hotbar->reset();
		tooltip->hide();
	}

	bool UIContext::click(int button, int x, int y, Modifiers modifiers) {
		refocusDialogs(x, y);

		if (dragging) {
			dragging = false;
			if (draggedWidget != nullptr && draggedWidgetActive) {
				draggedWidget->dragEnd(x, y, -1);
				draggedWidgetActive = false;
				setDraggedWidget(nullptr);
			}
		}

		return dispatch(&Widget::click, button, x, y, modifiers);
	}

	bool UIContext::mouseDown(int button, int x, int y, Modifiers modifiers) {
		return dispatch(&Widget::mouseDown, button, x, y, modifiers);
	}

	bool UIContext::mouseUp(int button, int x, int y, Modifiers modifiers) {
		if (auto pressed = getPressedWidget()) {
			return pressed->mouseUp(button, x, y, modifiers);
		}

		return dispatch(&Widget::mouseUp, button, x, y, modifiers);
	}

	bool UIContext::dragStart(int x, int y) {
		refocusDialogs(x, y);

		if (contextMenu && contextMenu->dragStart(x, y)) {
			return dragging = true;
		}

		if (autocompleteDropdown && autocompleteDropdown->dragStart(x, y)) {
			return dragging = true;
		}

		unfocusWidget();
		dragOrigin.emplace(x, y);

		for (const DialogPtr &dialog: reverse(dialogs)) {
			if (dialog->dragStart(x, y)) {
				return dragging = true;
			}
		}

		if (hotbar->contains(x, y)) {
			return dragging = hotbar->dragStart(x, y);
		}

		if (ClientGamePtr game = getGame()) {
			game->dragStart(x, y, window.getModifiers());
			return dragging = true;
		}

		return dragging = false;
	}

	bool UIContext::dragUpdate(int x, int y) {
		if (!dragging) {
			return false;
		}

		if (contextMenu && contextMenu->dragUpdate(x, y)) {
			return true;
		}

		if (autocompleteDropdown && autocompleteDropdown->dragUpdate(x, y)) {
			return true;
		}

		if (draggedWidget && dragOrigin != std::pair{x, y}) {
			draggedWidgetActive = true;
		}

		for (const WidgetPtr &widget: extraDragUpdaters) {
			if (widget->dragUpdate(x, y)) {
				return true;
			}
		}

		for (const DialogPtr &dialog: reverse(dialogs)) {
			if (dialog->dragUpdate(x, y)) {
				return true;
			}
		}

		if (hotbar->contains(x, y)) {
			return hotbar->dragUpdate(x, y);
		}

		if (ClientGamePtr game = getGame()) {
			game->dragUpdate(x, y, window.getModifiers());
			return true;
		}

		return false;
	}

	bool UIContext::dragEnd(int x, int y, double displacement) {
		Defer defer_game{[=, this] {
			if (ClientGamePtr game = getGame()) {
				game->dragEnd(x, y, window.getModifiers());
			}
		}};

		if (!dragging) {
			return false;
		}

		dragging = false;

		if (contextMenu && contextMenu->dragEnd(x, y, displacement)) {
			return true;
		}

		if (autocompleteDropdown && autocompleteDropdown->dragEnd(x, y, displacement)) {
			return true;
		}

		if (auto pressed = getPressedWidget()) {
			return pressed->dragEnd(x, y, displacement);
		}

		for (const WidgetPtr &widget: extraDragUpdaters) {
			widget->dragOrigin.reset();
			widget->dragEnd(x, y, displacement);
		}

		extraDragUpdaters.clear();

		bool out = false;

		for (const DialogPtr &dialog: reverse(dialogs)) {
			if (dialog->contains(x, y) && dialog->dragEnd(x, y, displacement)) {
				out = true;
				break;
			}
		}

		Defer defer;

		if (draggedWidget != nullptr && draggedWidgetActive) {
			draggedWidget->dragEnd(x, y, displacement);
			draggedWidgetActive = false;
			defer = [this] { setDraggedWidget(nullptr); };
		}

		if (!out && hotbar->contains(x, y)) {
			hotbar->dragEnd(x, y, displacement);
			return true;
		}

		return true;
	}

	bool UIContext::scroll(float x_delta, float y_delta, int x, int y, Modifiers modifiers) {
		return dispatch(&Widget::scroll, x_delta, y_delta, x, y, modifiers);
	}

	bool UIContext::keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) {
		if (auto focused = getFocusedWidget(); focused && focused->keyPressed(key, modifiers, is_repeat)) {
			return true;
		}

		if (auto context_menu = getContextMenu(); context_menu && context_menu->keyPressed(key, modifiers, is_repeat)) {
			return true;
		}

		if (DialogPtr dialog = focusedDialog.lock()) {
			if (dialog->keyPressed(key, modifiers, is_repeat)) {
				return true;
			}
		} else {
			for (const DialogPtr &dialog: reverse(dialogs)) {
				if (dialog->keyPressed(key, modifiers, is_repeat)) {
					return true;
				}
			}
		}

		return hotbar->keyPressed(key, modifiers, is_repeat);
	}

	bool UIContext::charPressed(uint32_t codepoint, Modifiers modifiers) {
		if (auto focused = getFocusedWidget(); focused && focused->charPressed(codepoint, modifiers)) {
			return true;
		}

		if (auto context_menu = getContextMenu(); context_menu && context_menu->charPressed(codepoint, modifiers)) {
			return true;
		}

		if (auto dialog = focusedDialog.lock()) {
			if (dialog->charPressed(codepoint, modifiers)) {
				return true;
			}
		} else {
			for (const DialogPtr &dialog: reverse(dialogs)) {
				if (dialog->charPressed(codepoint, modifiers)) {
					return true;
				}
			}
		}

		return hotbar->charPressed(codepoint, modifiers);
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

	RendererContext UIContext::getRenderers(float delta) const {
		return window.getRendererContext(delta);
	}

	void UIContext::focusWidget(const WidgetPtr &to_focus) {
		auto locked = focusedWidget.lock();

		if (to_focus == locked) {
			return;
		}

		if (locked)
			locked->onBlur();

		focusedWidget = to_focus;

		if (to_focus) {
			to_focus->onFocus();
		}
	}

	WidgetPtr UIContext::getFocusedWidget() const {
		return focusedWidget.lock();
	}

	void UIContext::unfocusWidget(const WidgetPtr &widget) {
		if (focusedWidget.lock() == widget) {
			unfocusWidget();
		}
	}

	void UIContext::unfocusWidget() {
		if (auto locked = focusedWidget.lock()) {
			locked->onBlur();
		}

		focusedWidget.reset();
	}

	void UIContext::focusDialog(const DialogPtr &to_focus) {
		auto locked = focusedDialog.lock();

		if (to_focus == locked) {
			return;
		}

		unfocusWidget();

		if (locked) {
			locked->onBlur();
		}

		focusedDialog = to_focus;

		if (to_focus) {
			to_focus->onFocus();
		}
	}

	DialogPtr UIContext::getFocusedDialog() const {
		return focusedDialog.lock();
	}

	void UIContext::unfocusDialog(const DialogPtr &dialog) {
		if (focusedDialog.lock() == dialog) {
			unfocusDialog();
		}
	}

	void UIContext::unfocusDialog() {
		if (auto locked = focusedDialog.lock()) {
			locked->onBlur();
		}

		focusedDialog.reset();
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

	std::optional<std::pair<double, double>> UIContext::getMouseCoordinates() const {
		return window.getMouseCoordinates<double>();
	}

	bool UIContext::checkMouse(const Rectangle &rectangle) const {
		if (const auto mouse = window.getMouseCoordinates<int>()) {
			const auto [x, y] = *mouse;
			return rectangle.contains(x, y);
		}

		return false;
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

	int UIContext::getWidth() const {
		return internalScissorStack.getBase().width;
	}

	int UIContext::getHeight() const {
		return internalScissorStack.getBase().height;
	}

	void UIContext::removeDialog(const DialogPtr &dialog) {
		for (auto iter = dialogs.begin(); iter != dialogs.end(); ++iter) {
			if (*iter == dialog) {
				(*iter)->onClose();
				dialogs.erase(iter);
				return;
			}
		}
	}

	void UIContext::addDialog(const DialogPtr &dialog) {
		dialogs.emplace_back(dialog);
		focusDialog(dialog);
	}

	const std::optional<std::pair<int, int>> & UIContext::getDragOrigin() const {
		return dragOrigin;
	}

	std::shared_ptr<ContextMenu> UIContext::getContextMenu() const {
		return contextMenu;
	}

	std::shared_ptr<Hotbar> UIContext::getHotbar() const {
		return hotbar;
	}

	std::shared_ptr<InventoryModule> UIContext::makePlayerInventoryModule() {
		if (ClientPlayerPtr player = getPlayer()) {
			return make<InventoryModule>(*this, 1, std::static_pointer_cast<ClientInventory>(player->getInventory(0)));
		}

		return make<InventoryModule>(*this, 1, std::shared_ptr<ClientInventory>{});
	}

	void UIContext::setScale(float new_scale) {
		scale = new_scale;
		iterateChildren([new_scale](const WidgetPtr &widget) {
			widget->rescale(new_scale);
		});
	}

	void UIContext::iterateChildren(const std::function<void(const WidgetPtr &)> &iterator) const {
		for (const DialogPtr &dialog: dialogs) {
			iterator(dialog);
		}

		auto visit = [&](const WidgetPtr &widget) {
			if (widget) {
				iterator(widget);
			}
		};

		visit(draggedWidget);
		visit(hotbar);
		visit(tooltip);
		visit(autocompleteDropdown);
		visit(contextMenu);
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

	void UIContext::refocusDialogs(int x, int y) {
		bool any_clicked = false;

		for (const DialogPtr &dialog: reverse(dialogs)) {
			if (dialog->contains(x, y)) {
				if (focusedDialog.lock() != dialog) {
					focusDialog(dialog);
				}
				any_clicked = true;
				break;
			}
		}

		if (!any_clicked) {
			focusDialog(nullptr);
		}
	}

	void UIContext::initUI() {
		reset();
		currentUI->init(window);
	}
}
