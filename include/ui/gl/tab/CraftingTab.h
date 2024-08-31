#pragma once

#include "ui/gl/tab/Tab.h"

#include <memory>

namespace Game3 {
	class ButtonWidget;
	class IconButtonWidget;
	class ProgressBarWidget;
	class TextInputWidget;

	class CraftingTab: public Tab {
		public:
			using Tab::Tab;

			void init() final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;

		private:
			std::shared_ptr<ProgressBarWidget> bar;
			std::shared_ptr<TextInputWidget> input;
			std::shared_ptr<ButtonWidget> button;
			std::shared_ptr<IconButtonWidget> iconButton;
	};
}
