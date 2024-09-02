#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/SettingsTab.h"
#include "ui/gl/widget/BoxWidget.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/GridWidget.h"
#include "ui/gl/widget/IconButtonWidget.h"
#include "ui/gl/widget/IconWidget.h"
#include "ui/gl/widget/LabelWidget.h"
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

		std::size_t row = 0;

		auto hostname_label = std::make_shared<LabelWidget>(scale);
		hostname_label->setText(ui, "Default Hostname");
		hostname_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(hostname_label, row, 0);

		auto hostname_input = std::make_shared<TextInputWidget>(scale);
		hostname_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		grid->attach(hostname_input, row, 1);

		++row;

		auto port_label = std::make_shared<LabelWidget>(scale);
		port_label->setText(ui, "Default Port");
		port_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(port_label, row, 0);

		auto port_input = std::make_shared<TextInputWidget>(scale);
		port_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		grid->attach(port_input, row, 1);

		++row;

		auto username_label = std::make_shared<LabelWidget>(scale);
		username_label->setText(ui, "Default Username");
		username_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(username_label, row, 0);

		auto username_input = std::make_shared<TextInputWidget>(scale);
		username_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		grid->attach(username_input, row, 1);

		++row;
	}

	void SettingsTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		scroller->render(ui, renderers, x, y, width, height);
	}

	void SettingsTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/settings.png"));
	}
}
