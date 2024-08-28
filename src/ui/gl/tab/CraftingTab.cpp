#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	void CraftingTab::render(UIContext &ui, RendererContext &renderers) {

		static float progress = 1;

		if (threadContext.random(0.0, 1.0) < 0.01) {
			progress = threadContext.random(0.0, 1.0);
		}

		constexpr float scale = SCALE;
		auto bar = std::make_shared<ProgressBarWidget>(scale, Color(1, 0, 0, 1), progress);
		bar->render(ui, renderers, 0, 0, scale * 100, scale * 10);
	}

	void CraftingTab::renderIcon(RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::click(int x, int y) {

	}
}
