#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/ContextMenu.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	void CraftingTab::init() {
		auto tab = shared_from_this();
		auto scroller = std::make_shared<Scroller>(ui, scale);
		scroller->insertAtEnd(tab);

		inventoryModule = ui.makePlayerInventoryModule();
		hbox = std::make_shared<Box>(ui, scale, Orientation::Horizontal);
		recipeList = std::make_shared<Box>(ui, scale, Orientation::Vertical);
		rightPane = std::make_shared<Box>(ui, scale, Orientation::Vertical);
	}

	void CraftingTab::render(const RendererContext &renderers, float x, float y, float width, float height) {
		maybeRemeasure(renderers, width, height);
		assert(firstChild != nullptr);
		Tab::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::onFocus() {
		if (resetQueued) {
			resetQueued = false;
			reset();
		}
	}

	void CraftingTab::reset() {
	}

	void CraftingTab::queueReset() {
		if (isActive()) {
			reset();
		} else {
			resetQueued = true;
		}
	}
}
