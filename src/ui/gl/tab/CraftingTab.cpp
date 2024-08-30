#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

namespace Game3 {
	void CraftingTab::render(UIContext &, const RendererContext &, float x, float y, float width, float height) {
		(void) x;
		(void) y;
		(void) width;
		(void) height;
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}
}
