#pragma once

#include "graphics/ScissorStack.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/Widget.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

namespace Game3 {
	class Canvas;
	class ClientGame;
	class Texture;
	struct RendererContext;

	class UIContext {
		private:
			Canvas &canvas;
			std::vector<std::unique_ptr<Dialog>> dialogs;
			ScissorStack internalScissorStack;
			std::map<std::filesystem::path, std::shared_ptr<Texture>> textureMap;
			std::shared_ptr<Widget> draggedWidget; // TODO

			template <typename T>
			static bool dialogMatcher(const std::unique_ptr<Dialog> &dialog) {
				return dynamic_cast<const T *>(dialog.get()) != nullptr;
			}

		public:
			ScissorStack scissorStack;

			UIContext(Canvas &);

			void render();
			void addDialog(std::unique_ptr<Dialog> &&);
			std::shared_ptr<ClientGame> getGame() const;

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
				dialogs.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			}
	};
}
