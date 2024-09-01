#pragma once

#include "ui/gl/tab/Tab.h"

#include <memory>

namespace Game3 {
	class BoxWidget;
	class ScrollerWidget;

	class SettingsTab: public Tab {
		public:
			using Tab::Tab;

			void init() final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;

		private:
			std::shared_ptr<ScrollerWidget> scroller;
			std::shared_ptr<BoxWidget> box;
	};
}
