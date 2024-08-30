#pragma once

#include "ui/gl/tab/Tab.h"

namespace Game3 {
	class CraftingTab: public Tab {
		public:
			using Tab::Tab;

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;
	};
}
