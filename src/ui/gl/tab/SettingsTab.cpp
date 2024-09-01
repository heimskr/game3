#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/SettingsTab.h"
#include "ui/gl/widget/BoxWidget.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/IconButtonWidget.h"
#include "ui/gl/widget/IconWidget.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/ScrollerWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	void SettingsTab::init() {
		auto tab = shared_from_this();
		scroller = std::make_shared<ScrollerWidget>(scale);
		box = std::make_shared<BoxWidget>(scale);
		scroller->insertAtEnd(tab);
		box->insertAtEnd(scroller);
	}

	void SettingsTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		scroller->render(ui, renderers, x, y, width, height);
	}

	void SettingsTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/settings.png"));
	}
}
