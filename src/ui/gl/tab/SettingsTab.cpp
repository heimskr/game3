#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/SettingsTab.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Checkbox.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/IconButton.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/ProgressBar.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/Slider.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <array>
#include <cassert>

namespace Game3 {
	void SettingsTab::init() {
		auto tab = shared_from_this();

		scroller = std::make_shared<Scroller>(scale);
		scroller->insertAtEnd(tab);

		grid = std::make_shared<Grid>(scale);
		grid->setSpacing(2 * scale);
		grid->insertAtEnd(scroller);

		std::size_t row = 0;

		auto hostname_label = std::make_shared<Label>(scale);
		hostname_label->setText(ui, "Default Hostname");
		hostname_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(hostname_label, row, 0);

		auto hostname_input = std::make_shared<TextInput>(scale);
		hostname_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		grid->attach(hostname_input, row, 1);

		++row;

		auto port_label = std::make_shared<Label>(scale);
		port_label->setText(ui, "Default Port");
		port_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(port_label, row, 0);

		auto port_input = std::make_shared<TextInput>(scale);
		port_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		grid->attach(port_input, row, 1);

		++row;

		auto username_label = std::make_shared<Label>(scale);
		username_label->setText(ui, "Default Username");
		username_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(username_label, row, 0);

		auto username_input = std::make_shared<TextInput>(scale);
		username_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		grid->attach(username_input, row, 1);

		++row;

		auto alert_label = std::make_shared<Label>(scale);
		alert_label->setText(ui, "Alert on Connect");
		alert_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(alert_label, row, 0);

		auto alert_checkbox = std::make_shared<Checkbox>(scale);
		alert_checkbox->setChecked(true);
		alert_checkbox->setFixedSize(scale * 8);
		grid->attach(alert_checkbox, row, 1);

		++row;

		auto lighting_label = std::make_shared<Label>(scale);
		lighting_label->setText(ui, "Render Lighting");
		lighting_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(lighting_label, row, 0);

		auto lighting_checkbox = std::make_shared<Checkbox>(scale);
		lighting_checkbox->setChecked(true);
		lighting_checkbox->setFixedSize(scale * 8);
		grid->attach(lighting_checkbox, row, 1);

		++row;

		auto timer_label = std::make_shared<Label>(scale);
		timer_label->setText(ui, "Timer Summaries");
		timer_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(timer_label, row, 0);

		auto timer_checkbox = std::make_shared<Checkbox>(scale);
		timer_checkbox->setChecked(true);
		timer_checkbox->setFixedSize(scale * 8);
		grid->attach(timer_checkbox, row, 1);

		++row;

		auto level_label = std::make_shared<Label>(scale);
		level_label->setText(ui, "Log Level");
		level_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(level_label, row, 0);

		auto level_slider = std::make_shared<Slider>(scale);
		level_slider->setFixedSize(100 * scale, 8 * scale);
		level_slider->setRange(0, 3);
		level_slider->setStep(1);
		grid->attach(level_slider, row, 1);

		auto level_value_label = std::make_shared<Label>(scale);
		level_value_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(level_value_label, row, 2);

		level_slider->setOnValueUpdate([this, weak_label = std::weak_ptr(level_value_label)](Slider &, double value) {
			if (auto label = weak_label.lock())
				label->setText(ui, std::format("{}", static_cast<int>(value)));
		});

		++row;
	}

	void SettingsTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		scroller->render(ui, renderers, x, y, width, height);
	}

	void SettingsTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/settings.png"));
	}
}
