#include "graphics/CircleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Spinner.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Spinner::Spinner(UIContext &ui, float selfScale, Color color, int count, float radius, float speed):
		Widget(ui, selfScale),
		color(color),
		count(count),
		radius(radius),
		speed(speed) {}

	void Spinner::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (fixedHeight >= 0) {
			height = fixedHeight;
		}

		if (fixedWidth >= 0) {
			width = fixedWidth;
		}

		const float original_width = width;
		const float original_height = height;
		float dummy{};
		measure(renderers, Orientation::Horizontal, original_width, original_height, dummy, width);
		measure(renderers, Orientation::Vertical,   original_width, original_height, dummy, height);

		Widget::render(renderers, x, y, width, height);

		if (shouldCull()) {
			return;
		}

		const auto scale = getScale();
		const auto scaled_radius = radius * scale;

		rotation += renderers.delta * speed;

		for (int i = 0; i < count; ++i) {
			float radians = (static_cast<float>(i) / count + rotation) * 2.f * M_PIf;
			float x_offset = width * (0.5f + std::cos(radians) / 2.f) + scaled_radius;
			float y_offset = height * (0.5f + std::sin(radians) / 2.f) + scaled_radius;
			renderers.circle.drawOnScreen(color, x + x_offset, y + y_offset, scaled_radius, scaled_radius);
		}
	}

	SizeRequestMode Spinner::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Spinner::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		float size{};

		if (orientation == Orientation::Horizontal) {
			if (fixedWidth >= 0) {
				size = fixedWidth * ui.scale;
			} else {
				size = getScale() * 16;
			}
		} else {
			if (fixedHeight >= 0) {
				size = fixedHeight * ui.scale;
			} else {
				size = getScale() * 16;
			}
		}

		minimum = natural = size;
	}
}
