#pragma once

#include "graphics/ScissorStack.h"
#include "ui/gl/dialog/Dialog.h"
#include "ui/gl/widget/Widget.h"
#include "ui/Modifiers.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace Game3 {
	class AutocompleteDropdown;
	class ClientGame;
	class ClientPlayer;
	class ContextMenu;
	class Hotbar;
	class InventoryModule;
	class Texture;
	class Tooltip;
	class Window;
	struct RendererContext;

	class UIContext {
		public:
			Window &window;
			ScissorStack scissorStack;
			bool renderingDraggedWidget = false;

			UIContext(Window &);

			void render(float mouse_x, float mouse_y);
			std::shared_ptr<ClientGame> getGame() const;
			void onResize(int x, int y);
			void reset();
			/** Returns true iff the click accomplished something. */
			bool click(int button, int x, int y);
			bool mouseDown(int button, int x, int y);
			bool mouseUp(int button, int x, int y);
			bool dragStart(int x, int y);
			bool dragUpdate(int x, int y);
			bool dragEnd(int x, int y);
			bool scroll(float x_delta, float y_delta, int x, int y);
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat);
			bool charPressed(uint32_t character, Modifiers);
			void setDraggedWidget(WidgetPtr);
			WidgetPtr getDraggedWidget() const;
			std::shared_ptr<ClientPlayer> getPlayer() const;
			RendererContext getRenderers() const;
			void focusWidget(const WidgetPtr &);
			WidgetPtr getFocusedWidget() const;
			/** Unfocuses if the currently focused widget is the given widget. */
			void unfocusWidget(const WidgetPtr &);
			void unfocusWidget();
			void focusDialog(const DialogPtr &);
			DialogPtr getFocusedDialog() const;
			/** Unfocuses if the currently focused dialog is the given dialog. */
			void unfocusDialog(const DialogPtr &);
			void unfocusDialog();
			void setPressedWidget(const WidgetPtr &);
			WidgetPtr getPressedWidget() const;
			void setAutocompleteDropdown(std::shared_ptr<AutocompleteDropdown>);
			std::shared_ptr<AutocompleteDropdown> getAutocompleteDropdown() const;
			void unpress();
			std::pair<double, double> getMouseCoordinates() const;
			bool checkMouse(const Rectangle &) const;
			std::shared_ptr<Tooltip> getTooltip() const;
			void addDragUpdater(WidgetPtr);
			bool anyDragUpdaters() const;
			void setContextMenu(std::shared_ptr<ContextMenu>);
			int getWidth() const;
			int getHeight() const;
			void removeDialog(const DialogPtr &);
			void addDialog(const DialogPtr &);
			const std::optional<std::pair<int, int>> & getDragOrigin() const;
			std::shared_ptr<ContextMenu> getContextMenu() const;
			std::shared_ptr<Hotbar> getHotbar() const;
			std::shared_ptr<InventoryModule> makePlayerInventoryModule();

			/** Order: clockwise starting at top left. */
			void drawFrame(const RendererContext &, double scale, bool alpha, const std::array<std::string_view, 8> &, const Color &interior = {0, 0, 0, 0});

			template <typename... Ts>
			size_t removeDialogs() {
				return (std::erase_if(dialogs, &dialogMatcher<Ts>), ...);
			}

			template <typename T>
			bool hasDialog() const {
				return std::ranges::any_of(dialogs, &dialogMatcher<T>);
			}

			template <typename T, typename... Args>
			void emplaceDialog(Args &&...args) {
				auto dialog = std::make_shared<T>(*this, std::forward<Args>(args)...);
				dialog->init();
				dialogs.emplace_back(dialog);
				focusDialog(dialog);
			}

		private:
			std::vector<std::shared_ptr<Dialog>> dialogs;
			ScissorStack internalScissorStack; // TODO: remove this
			WidgetPtr draggedWidget;
			bool draggedWidgetActive = false;
			std::optional<std::pair<int, int>> dragOrigin;
			std::shared_ptr<Hotbar> hotbar;
			std::shared_ptr<Tooltip> tooltip;
			std::shared_ptr<AutocompleteDropdown> autocompleteDropdown;
			std::shared_ptr<ContextMenu> contextMenu;
			WeakWidgetPtr focusedWidget;
			WeakWidgetPtr pressedWidget;
			WeakDialogPtr focusedDialog;
			std::set<WidgetPtr> extraDragUpdaters;

			void refocusDialogs(int x, int y);

			template <typename T>
			static bool dialogMatcher(const std::shared_ptr<Dialog> &dialog) {
				if (T *cast = dynamic_cast<T *>(dialog.get())) {
					cast->onClose();
					return true;
				}

				return false;
			}
	};
}
