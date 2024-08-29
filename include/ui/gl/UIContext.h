#pragma once

#include "graphics/ScissorStack.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/widget/HotbarWidget.h"
#include "ui/gl/widget/Widget.h"

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
			void setDraggedWidget(WidgetPtr);
			WidgetPtr getDraggedWidget() const;
			std::shared_ptr<ClientPlayer> getPlayer() const;

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
			Canvas &canvas;
			std::vector<std::shared_ptr<Dialog>> dialogs;
			ScissorStack internalScissorStack; // TODO: remove this
			WidgetPtr draggedWidget;
			bool draggedWidgetActive = false;
			std::optional<std::pair<int, int>> dragOrigin;
			std::unique_ptr<HotbarWidget> hotbarWidget;

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
