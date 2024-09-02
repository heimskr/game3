#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Checkbox.h"

namespace {
	constexpr Game3::Color DEFAULT_PRIMARY_COLOR{"#926641"};
	constexpr Game3::Color DEFAULT_INTERIOR_COLOR{"#ffffff"};
}

namespace Game3 {
	Checkbox::Checkbox(float scale, Color top_color, Color bottom_color, Color check_color, Color interior_color):
		Widget(scale), topColor(top_color), bottomColor(bottom_color), checkColor(check_color), interiorColor(interior_color) {}

	Checkbox::Checkbox(float scale, Color primary_color, Color interior_color):
		Checkbox(scale, primary_color, primary_color.darken(), primary_color.darken(), interior_color) {}

	Checkbox::Checkbox(float scale):
		Checkbox(scale, DEFAULT_PRIMARY_COLOR, DEFAULT_INTERIOR_COLOR) {}

	void Checkbox::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		fixSizes(width, height);
		Widget::render(ui, renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;

		const float top_fraction = getTopFraction();
		const float bottom_fraction = 1 - top_fraction;

		rectangler.drawOnScreen(topColor, x, y, width, height * top_fraction);
		rectangler.drawOnScreen(bottomColor, x, y + height * top_fraction, width, height * bottom_fraction);
		rectangler.drawOnScreen(interiorColor, x + scale, y + scale, width - 2 * scale, height - 2 * scale);

		if (checked) {
			rectangler.drawOnScreen(checkColor, x + width * 0.225, y + height * 0.525, width * 0.3, height / 8, 45);
			rectangler.drawOnScreen(checkColor, x + width * 0.325, y + height * 0.45,  width * 0.5, height / 8, -45);
		}
	}

	bool Checkbox::click(UIContext &, int, int, int) {
		checked = !checked;
		return true;
	}

	SizeRequestMode Checkbox::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Checkbox::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			if (0 < fixedWidth)
				minimum = natural = fixedWidth;
			else
				minimum = natural = for_width;
		} else {
			if (0 < fixedHeight)
				minimum = natural = fixedHeight;
			else
				minimum = natural = for_height;
		}
	}

	bool Checkbox::getChecked() const {
		return checked;
	}

	void Checkbox::setChecked(bool new_checked) {
		checked = new_checked;
	}

	float Checkbox::getTopFraction() const {
		return 0.6;
	}
}
