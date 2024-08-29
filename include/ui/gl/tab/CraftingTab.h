#pragma once

#include "ui/gl/tab/Tab.h"

namespace Game3 {
	class Module;

	class CraftingTab: public Tab {
		public:
			using Tab::Tab;

			void render(UIContext &, const RendererContext &) final;
			void renderIcon(const RendererContext &) final;
			void click(int button, int x, int y) final;
	};
}
