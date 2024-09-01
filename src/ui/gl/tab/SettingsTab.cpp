#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/SettingsTab.h"
#include "ui/gl/widget/BoxWidget.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/GridWidget.h"
#include "ui/gl/widget/IconButtonWidget.h"
#include "ui/gl/widget/IconWidget.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/ScrollerWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <array>
#include <cassert>

namespace Game3 {
	void SettingsTab::init() {
		auto tab = shared_from_this();

		scroller = std::make_shared<ScrollerWidget>(scale);
		scroller->insertAtEnd(tab);

		grid = std::make_shared<GridWidget>(scale);
		grid->setSpacing(2 * scale);
		grid->insertAtEnd(scroller);

		std::array labels{
			std::array{"Hello", "World", "!"},
			std::array{"Foo", "Bar and Baz", "Quux"},
		};

		for (std::size_t row = 0; row < labels.size(); ++row) {
			for (std::size_t column = 0; column < labels[row].size(); ++column) {
				const auto &label = labels[row][column];
				if (label) {
					auto button = std::make_shared<ButtonWidget>(scale);
					button->setText(label);
					grid->attach(button, row, column);
				}
			}
		}
	}

	void SettingsTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		scroller->render(ui, renderers, x, y, width, height);
	}

	void SettingsTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/settings.png"));
	}
}
