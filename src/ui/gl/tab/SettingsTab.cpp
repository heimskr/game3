#include "graphics/OpenGL.h"
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
#include "ui/Window.h"
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
	void SettingsTab::init() {
		auto tab = shared_from_this();

		auto &settings = ui.getRenderers().settings;
		auto settings_lock = settings.sharedLock();

		scroller = std::make_shared<Scroller>(ui, selfScale);
		scroller->insertAtEnd(tab);

		grid = std::make_shared<Grid>(ui, selfScale);
		grid->setSpacing(2 * selfScale);
		grid->insertAtEnd(scroller);

		std::size_t row = 0;

		auto add_checkbox = [&](UString label_text, bool ClientSettings::*member, bool invert = false) {
			auto label = std::make_shared<Label>(ui, selfScale, std::move(label_text));
			label->setVerticalAlignment(Alignment::Center);
			grid->attach(label, row, 0);

			auto checkbox = std::make_shared<Checkbox>(ui, selfScale);
			checkbox->setChecked(invert? !(settings.*member) : settings.*member);
			checkbox->setFixedSize(selfScale * 8);
			checkbox->onCheck.connect([this, member, invert](bool checked) {
				applySetting(member, invert? !checked : checked);
			});
			grid->attach(checkbox, row, 1);

			++row;

			return checkbox;
		};

		auto add_slider = [&](UString label_text, UString tooltip = {}) {
			auto label = std::make_shared<Label>(ui, selfScale);
			label->setText(std::move(label_text));
			label->setVerticalAlignment(Alignment::Center);
			grid->attach(label, row, 0);

			if (!tooltip.empty()) {
				label->setTooltipText(std::move(tooltip));
			}

			auto slider = std::make_shared<Slider>(ui, selfScale);
			slider->setFixedSize(100 * selfScale, 8 * selfScale);

			auto value_label = std::make_shared<Label>(ui, selfScale);
			value_label->setVerticalAlignment(Alignment::Center);

			auto hbox = std::make_shared<Box>(ui, selfScale, Orientation::Horizontal, 0, 0, Color{});
			hbox->append(slider);
			hbox->append(value_label);
			grid->attach(hbox, row, 1);

			slider->onValueUpdate.connect([weak_label = std::weak_ptr(value_label)](Slider &slider, double) {
				if (auto label = weak_label.lock()) {
					label->setText(slider.getTooltipText());
				}
			});

			++row;

			return slider;
		};

		auto hostname_label = std::make_shared<Label>(ui, selfScale);
		hostname_label->setText("Default Hostname");
		hostname_label->setVerticalAlignment(Alignment::Center);
		grid->attach(hostname_label, row, 0);

		auto hostname_input = std::make_shared<TextInput>(ui, selfScale);
		hostname_input->setFixedHeight(selfScale * TEXT_INPUT_HEIGHT_FACTOR);
		hostname_input->setText(settings.hostname);
		hostname_input->setHorizontalExpand(true);
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

		auto port_label = std::make_shared<Label>(ui, selfScale);
		port_label->setText("Default Port");
		port_label->setVerticalAlignment(Alignment::Center);
		grid->attach(port_label, row, 0);

		auto port_input = std::make_shared<TextInput>(ui, selfScale);
		port_input->setFixedSize(100 * selfScale, selfScale * TEXT_INPUT_HEIGHT_FACTOR);
		port_input->setText(std::to_string(settings.port));
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

		auto username_label = std::make_shared<Label>(ui, selfScale);
		username_label->setText("Default Username");
		username_label->setVerticalAlignment(Alignment::Center);
		grid->attach(username_label, row, 0);

		auto username_input = std::make_shared<TextInput>(ui, selfScale);
		username_input->setFixedSize(100 * selfScale, selfScale * TEXT_INPUT_HEIGHT_FACTOR);
		username_input->setText(settings.username);
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

		add_checkbox("Alert on Connect", &ClientSettings::alertOnConnection);
		add_checkbox("Render Lighting", &ClientSettings::renderLighting);
		add_checkbox("Enable Timers", &ClientSettings::hideTimers, true);
		add_checkbox("Display FPS", &ClientSettings::showFPS);
		add_checkbox("Cap FPS", &ClientSettings::capFPS);
		add_checkbox("Special Effects", &ClientSettings::specialEffects);

		auto mystery_slider = add_slider("Mystery");
		mystery_slider->setRange(0, 40);
		mystery_slider->setStep(0.00000000001);
		mystery_slider->setValue(settings.mystery);
		mystery_slider->setDisplayDigits(5);
		mystery_slider->onValueUpdate.connect([this](Slider &, double value) {
			applySetting(&ClientSettings::mystery, value);
		});

		auto scale_slider = add_slider("UI Scale");
		scale_slider->setRange(0.5, 16);
		scale_slider->setStep(0.5);
		scale_slider->setValue(settings.uiScale);
		scale_slider->setDisplayDigits(1);
		scale_slider->onRelease.connect([this](Slider &, double value) {
			applySetting(&ClientSettings::uiScale, value);
		});

		auto level_slider = add_slider("Log Level");
		level_slider->setRange(0, 3);
		level_slider->setStep(1);
		level_slider->setValue(Logger::level);
		level_slider->onValueUpdate.connect([this](Slider &, double value) {
			applySetting(&ClientSettings::logLevel, value);
		});

		auto frequency_slider = add_slider("Tick Frequency");
		frequency_slider->setRange(1, 240);
		frequency_slider->setStep(1);
		frequency_slider->setDisplayDigits(0);
		frequency_slider->setValue(settings.tickFrequency);
		frequency_slider->onValueUpdate.connect([&](Slider &, double value) {
			applySetting(&ClientSettings::tickFrequency, value);
		});

		auto smoothing_slider = add_slider("FPS Accumulation", "Affects the FPS counter,\nnot the FPS itself.");
		smoothing_slider->setRange(1, 1000);
		smoothing_slider->setStep(1);
		smoothing_slider->setDisplayDigits(0);
		smoothing_slider->setValue(settings.fpsSmoothing);
		smoothing_slider->onValueUpdate.connect([&](Slider &, double value) {
			applySetting(&ClientSettings::fpsSmoothing, value);
		});

		auto save_button = std::make_shared<Button>(ui, selfScale);
		save_button->setText("Save");
		save_button->setFixedHeight(10 * selfScale);
		save_button->setOnClick([this](Widget &, int, int, int) {
			saveSettings();
			return true;
		});
		grid->attach(save_button, row++, 0);
	}

	void SettingsTab::render(const RendererContext &renderers, float x, float y, float width, float height) {
		scroller->render(renderers, x, y, width, height);
	}

	void SettingsTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/settings.png"));
	}

	void SettingsTab::saveSettings() {
		ui.window.saveSettings();
	}
}
