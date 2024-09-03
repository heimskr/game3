#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	void CraftingTab::init() {
		auto tab = shared_from_this();
		auto scroller = std::make_shared<Scroller>(scale);
		scroller->insertAtEnd(tab);
	}

	void CraftingTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		WidgetPtr child = firstChild;
		assert(child != nullptr);
		child->render(ui, renderers, x, y, width, height);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}
}
