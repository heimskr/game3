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
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

#include <array>
#include <cassert>

namespace {
	using namespace Game3;

	Color getInvalidColor() {
		auto hsv = DEFAULT_TEXTINPUT_INTERIOR_COLOR.convert<OKHsv>();
		hsv.hue = 0;
		hsv.saturation = 0.2;
		return hsv.convert<Color>();
	}
}

namespace Game3 {
	void SettingsTab::init(UIContext &) {
		auto tab = shared_from_this();

		auto &settings = ui.getRenderers().settings;
		auto settings_lock = settings.sharedLock();

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
		hostname_input->setText(ui, settings.hostname);
		hostname_input->onChange.connect([this](TextInput &input, const UString &text) {
			if (text.empty()) {
				input.setInteriorColor(getInvalidColor());
				return;
			}

			input.setInteriorColor();
			applySetting(&ClientSettings::hostname, text.raw());
		});
		grid->attach(hostname_input, row, 1);

		++row;

		auto port_label = std::make_shared<Label>(scale);
		port_label->setText(ui, "Default Port");
		port_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(port_label, row, 0);

		auto port_input = std::make_shared<TextInput>(scale);
		port_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		port_input->setText(ui, std::to_string(settings.port));
		port_input->onChange.connect([this](TextInput &input, const UString &text) {
			uint16_t port{};

			Color color = DEFAULT_TEXTINPUT_INTERIOR_COLOR;

			try {
				port = parseNumber<uint16_t>(text.raw());
				applySetting(&ClientSettings::port, port);
			} catch (const std::invalid_argument &) {
				auto hsv = color.convert<OKHsv>();
				hsv.hue = 0;
				hsv.saturation = 0.3;
				color = hsv.convert<Color>();
			}

			input.setInteriorColor(color);
		});
		grid->attach(port_input, row, 1);

		++row;

		auto username_label = std::make_shared<Label>(scale);
		username_label->setText(ui, "Default Username");
		username_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(username_label, row, 0);

		auto username_input = std::make_shared<TextInput>(scale);
		username_input->setFixedSize(100 * scale, scale * TEXT_INPUT_HEIGHT_FACTOR);
		username_input->setText(ui, settings.username);
		username_input->onChange.connect([this](TextInput &input, const UString &text) {
			if (text.empty()) {
				input.setInteriorColor(getInvalidColor());
				return;
			}

			input.setInteriorColor();
			applySetting(&ClientSettings::username, text.raw());
		});
		grid->attach(username_input, row, 1);

		++row;

		auto alert_label = std::make_shared<Label>(scale);
		alert_label->setText(ui, "Alert on Connect");
		alert_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(alert_label, row, 0);

		auto alert_checkbox = std::make_shared<Checkbox>(scale);
		alert_checkbox->setChecked(settings.alertOnConnection);
		alert_checkbox->setFixedSize(scale * 8);
		alert_checkbox->onCheck.connect([this](bool checked) {
			applySetting(&ClientSettings::alertOnConnection, checked);
		});
		grid->attach(alert_checkbox, row, 1);

		++row;

		auto lighting_label = std::make_shared<Label>(scale);
		lighting_label->setText(ui, "Render Lighting");
		lighting_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(lighting_label, row, 0);

		auto lighting_checkbox = std::make_shared<Checkbox>(scale);
		lighting_checkbox->setChecked(settings.renderLighting);
		lighting_checkbox->setFixedSize(scale * 8);
		lighting_checkbox->onCheck.connect([this](bool checked) {
			applySetting(&ClientSettings::renderLighting, checked);
		});
		grid->attach(lighting_checkbox, row, 1);

		++row;

		auto timer_label = std::make_shared<Label>(scale);
		timer_label->setText(ui, "Enable Timers");
		timer_label->setVerticalAlignment(Alignment::Middle);
		grid->attach(timer_label, row, 0);

		auto timer_checkbox = std::make_shared<Checkbox>(scale);
		timer_checkbox->setChecked(!settings.hideTimers);
		timer_checkbox->setFixedSize(scale * 8);
		timer_checkbox->onCheck.connect([this](bool checked) {
			applySetting(&ClientSettings::hideTimers, !checked);
		});
		grid->attach(timer_checkbox, row, 1);

		++row;

		auto add_slider = [&](UString label_text) {
			auto label = std::make_shared<Label>(scale);
			label->setText(ui, std::move(label_text));
			label->setVerticalAlignment(Alignment::Middle);
			grid->attach(label, row, 0);

			auto slider = std::make_shared<Slider>(scale);
			slider->setFixedSize(100 * scale, 8 * scale);
			grid->attach(slider, row, 1);

			auto value_label = std::make_shared<Label>(scale);
			value_label->setVerticalAlignment(Alignment::Middle);
			grid->attach(value_label, row, 2);

			slider->onValueUpdate.connect([this, weak_label = std::weak_ptr(value_label)](Slider &slider, double) {
				if (auto label = weak_label.lock())
					label->setText(ui, slider.getTooltipText());
			});

			++row;

			return slider;
		};

		auto level_slider = add_slider("Log Level");
		level_slider->setRange(0, 3);
		level_slider->setStep(1);
		level_slider->setValue(Logger::level);
		level_slider->onValueUpdate.connect([this](Slider &, double value) {
			applySetting(&ClientSettings::logLevel, value);
		});

		auto divisor_slider = add_slider("Size Divisor");
		divisor_slider->setRange(-.5, 4);
		divisor_slider->setStep(.1);
		divisor_slider->setValue(settings.sizeDivisor);
		divisor_slider->setDisplayDigits(1);
		divisor_slider->onValueUpdate.connect([this](Slider &, double value) {
			applySetting(&ClientSettings::sizeDivisor, value);
		});

		auto frequency_slider = add_slider("Tick Frequency");
		frequency_slider->setRange(1, 240);
		frequency_slider->setStep(1);
		frequency_slider->setDisplayDigits(0);
		frequency_slider->setValue(settings.tickFrequency);
		frequency_slider->onValueUpdate.connect([&](Slider &, double value) {
			applySetting(&ClientSettings::tickFrequency, value);
		});

		auto save_button = std::make_shared<Button>(scale);
		save_button->setText("Save");
		save_button->setFixedHeight(10 * scale);
		save_button->setOnClick([this](Widget &, UIContext &, int, int, int) {
			saveSettings();
			return true;
		});
		grid->attach(save_button, row++, 0);
	}

	void SettingsTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		scroller->render(ui, renderers, x, y, width, height);
	}

	void SettingsTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/settings.png"));
	}

	void SettingsTab::saveSettings() {
		ui.canvas.window.saveSettings();
	}
}
