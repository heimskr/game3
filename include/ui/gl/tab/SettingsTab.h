#pragma once

#include "ui/gl/tab/Tab.h"

#include <memory>

namespace Game3 {
	class Grid;
	class Scroller;

	class SettingsTab: public Tab {
		public:
			using Tab::Tab;

			void init() final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;

		private:
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Grid> grid;
	};
}
