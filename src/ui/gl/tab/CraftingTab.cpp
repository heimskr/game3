#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	void CraftingTab::render(UIContext &ui, RendererContext &renderers) {

	}

	void CraftingTab::renderIcon(RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::click(int x, int y) {

	}
}
