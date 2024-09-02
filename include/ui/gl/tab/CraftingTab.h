#pragma once

#include "ui/gl/tab/Tab.h"

#include <memory>

namespace Game3 {
	class Box;
	class Button;
	class IconButton;
	class Icon;
	class ProgressBar;
	class TextInput;

	class CraftingTab: public Tab {
		public:
			using Tab::Tab;

			void init() final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;

		private:
			std::shared_ptr<Box> box;
			std::shared_ptr<ProgressBar> bar;
			std::shared_ptr<TextInput> input;
			std::shared_ptr<Button> button;
			std::shared_ptr<IconButton> iconButton;
			std::shared_ptr<Icon> icon;
	};
}
