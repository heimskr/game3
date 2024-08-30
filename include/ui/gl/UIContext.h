#pragma once

#include "graphics/ScissorStack.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/widget/HotbarWidget.h"
#include "ui/gl/widget/Widget.h"
#include "ui/Modifiers.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace Game3 {
	class Canvas;
	class ClientGame;
	class ClientPlayer;
	class HotbarWidget;
	class Texture;
	struct RendererContext;

	class UIContext {
		public:
			Canvas &canvas;
			ScissorStack scissorStack;
			bool renderingDraggedWidget = false;

			UIContext(Canvas &);

			void render(float mouse_x, float mouse_y);
			void addDialog(std::shared_ptr<Dialog>);
			std::shared_ptr<ClientGame> getGame() const;
			void onResize(int x, int y);
			/** Returns true iff the click accomplished something. */
			bool click(int button, int x, int y);
			bool dragStart(int x, int y);
			bool dragUpdate(int x, int y);
			bool dragEnd(int x, int y);
			bool scroll(float x_delta, float y_delta, int x, int y);
			bool keyPressed(uint32_t character, Modifiers);
			void setDraggedWidget(WidgetPtr);
			WidgetPtr getDraggedWidget() const;
			std::shared_ptr<ClientPlayer> getPlayer() const;
			RendererContext getRenderers() const;
			void focusWidget(std::weak_ptr<Widget>);
			WidgetPtr getFocusedWidget() const;
			void unfocus();
			std::pair<double, double> getAbsoluteMouseCoordinates() const;
			std::pair<double, double> getRelativeMouseCoordinates() const;

			template <typename T>
			size_t removeDialogs() {
				return std::erase_if(dialogs, &dialogMatcher<T>);
			}

			template <typename T>
			bool hasDialog() const {
				return std::ranges::any_of(dialogs, &dialogMatcher<T>);
			}

			template <typename T, typename... Args>
			void addDialog(Args &&...args) {
				dialogs.emplace_back(std::make_shared<T>(*this, std::forward<Args>(args)...));
			}

		private:
			std::vector<std::shared_ptr<Dialog>> dialogs;
			ScissorStack internalScissorStack; // TODO: remove this
			WidgetPtr draggedWidget;
			bool draggedWidgetActive = false;
			std::optional<std::pair<int, int>> dragOrigin;
			std::unique_ptr<HotbarWidget> hotbarWidget;
			std::weak_ptr<Widget> focusedWidget;

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
