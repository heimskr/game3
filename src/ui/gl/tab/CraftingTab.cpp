#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
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

		auto vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical);
		vbox->insertAtEnd(scroller);

		auto input = std::make_shared<TextInput>(ui, scale);
		input->setHorizontalExpand(true);
		input->setSuggestions(std::vector<UString>{
			"foo",
			"bar",
			"ba",
			"baz",
			"apple",
			"application",
		});

		input->setOnClick([this](Widget &input, int button, int mouse_x, int mouse_y) {
			if (button != 3)
				return false;

			auto menu = std::make_shared<ContextMenu>(ui, scale, input.shared_from_this(), mouse_x, mouse_y);

			for (std::string str: {"Hello", "World!", "Foo", "Bar", "Baz"}) {
				menu->addItem(std::make_shared<ContextMenuItem>(ui, scale, str, [str] { INFO("{}", str); }));
			}

			menu->maybeRemeasure(ui.getRenderers(), -2, -2);

			ui.setContextMenu(std::move(menu));

			return true;
		});

		input->insertAtEnd(vbox);
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
}
