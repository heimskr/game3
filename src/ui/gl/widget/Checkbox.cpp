#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Checkbox.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr Game3::Color DEFAULT_PRIMARY_COLOR{"#926641"};
	constexpr Game3::Color DEFAULT_INTERIOR_COLOR{"#ffffff"};
}

namespace Game3 {
	Checkbox::Checkbox(UIContext &ui, float selfScale, Color top_color, Color bottom_color, Color check_color, Color interior_color):
		Widget(ui, selfScale), topColor(top_color), bottomColor(bottom_color), checkColor(check_color), interiorColor(interior_color) {}

	Checkbox::Checkbox(UIContext &ui, float selfScale, Color primary_color, Color interior_color):
		Checkbox(ui, selfScale, primary_color, primary_color.darken(), primary_color.darken(), interior_color) {}

	Checkbox::Checkbox(UIContext &ui, float selfScale):
		Checkbox(ui, selfScale, DEFAULT_PRIMARY_COLOR, DEFAULT_INTERIOR_COLOR) {}

	void Checkbox::render(const RendererContext &renderers, float x, float y, float width, float height) {
		fixSizes(width, height, ui.scale);
		Widget::render(renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;

		const float top_fraction = getTopFraction();
		const float bottom_fraction = 1 - top_fraction;
		const auto scale = getScale();

		rectangler.drawOnScreen(topColor, x, y, width, height * top_fraction);
		rectangler.drawOnScreen(bottomColor, x, y + height * top_fraction, width, height * bottom_fraction);
		rectangler.drawOnScreen(interiorColor, x + scale, y + scale, width - 2 * scale, height - 2 * scale);

		if (getChecked()) {
			rectangler.drawOnScreen(checkColor, x + width * 0.225, y + height * 0.525, width * 0.3, height / 8, 45);
			rectangler.drawOnScreen(checkColor, x + width * 0.325, y + height * 0.45,  width * 0.5, height / 8, -45);
		}
	}

	bool Checkbox::click(int, int, int, Modifiers) {
		setChecked(!getChecked());
		return true;
	}

	SizeRequestMode Checkbox::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Checkbox::measure(const RendererContext &, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			if (0 < fixedWidth) {
				minimum = natural = fixedWidth * ui.scale;
			} else {
				minimum = natural = for_width;
			}
		} else {
			if (0 < fixedHeight) {
				minimum = natural = fixedHeight * ui.scale;
			} else {
				minimum = natural = for_height;
			}
		}
	}

	bool Checkbox::getChecked() const {
		return checked;
	}

	void Checkbox::setChecked(bool new_checked) {
		checked = new_checked;
		onCheck(new_checked);
	}

	float Checkbox::getTopFraction() const {
		return 0.6;
	}
}
