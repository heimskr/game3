#include "graphics/Color.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	using namespace Game3;

	constexpr Color DEFAULT_EXTERIOR_COLOR{0.6, 0.3, 0.0, 1.0};
	constexpr Color DEFAULT_BACKGROUND_COLOR{0.25, 0.0, 0.0, 1.0};

	inline Color darken(Color color) {
		RGB dark = RGB{color.red, color.green, color.blue}.darken();
		return Color{dark.r, dark.g, dark.b, color.alpha};
	}
}

namespace Game3 {
	ProgressBarWidget::ProgressBarWidget(float fixed_height, float scale, Color interior_color, Color background_color, Color exterior_color, float progress):
		fixedHeight(fixed_height),
		scale(scale),
		topInteriorColor(interior_color),
		bottomInteriorColor(darken(topInteriorColor)),
		backgroundColor(background_color),
		topExteriorColor(exterior_color),
		bottomExteriorColor(darken(topExteriorColor)),
		progress(progress) {}

	ProgressBarWidget::ProgressBarWidget(float fixed_height, float scale, Color interior_color, float progress):
		ProgressBarWidget(fixed_height, scale, interior_color, DEFAULT_BACKGROUND_COLOR, DEFAULT_EXTERIOR_COLOR, progress) {}

	void ProgressBarWidget::render(UIContext &ui, RendererContext &renderers, float x, float y, float width, float height) {
		RectangleRenderer &rectangle = renderers.rectangle;

		if (fixedHeight > 0)
			height = fixedHeight;

		Widget::render(ui, renderers, x, y, width, height);

		const float bar_width = progress * (width - 2 * scale);
		const float top_height = .6f * (height - 2 * scale);
		const float bottom_height = .4f * (height - 2 * scale);

		// Bar fill
		rectangle.drawOnScreen(topInteriorColor, x + scale, y + scale, bar_width, top_height);
		rectangle.drawOnScreen(bottomInteriorColor, x + scale, y + scale + top_height, bar_width, bottom_height);

		// Background
		rectangle.drawOnScreen(backgroundColor, x + scale + bar_width, y + scale, width - 2 * scale - bar_width, height - 2 * scale);

		// Top and bottom horizontal rectangles
		rectangle.drawOnScreen(topExteriorColor, x + scale, y, width - 2 * scale, scale);
		rectangle.drawOnScreen(bottomExteriorColor, x + scale, y + height - scale, width - 2 * scale, scale);

		// Corner rectangles
		rectangle.drawOnScreen(topExteriorColor, x + scale, y + scale, scale, scale);
		rectangle.drawOnScreen(topExteriorColor, x + width - scale * 2, y + scale, scale, scale);
		rectangle.drawOnScreen(bottomExteriorColor, x + scale, y + height - scale * 2, scale, scale);
		rectangle.drawOnScreen(bottomExteriorColor, x + width - scale * 2, y + height - scale * 2, scale, scale);

		// Side rectangles
		rectangle.drawOnScreen(topExteriorColor, x, y + scale, scale, top_height);
		rectangle.drawOnScreen(topExteriorColor, x + width - scale, y + scale, scale, top_height);
		rectangle.drawOnScreen(bottomExteriorColor, x, y + scale + top_height, scale, bottom_height);
		rectangle.drawOnScreen(bottomExteriorColor, x + width - scale, y + scale  + top_height, scale, bottom_height);
	}

	float ProgressBarWidget::calculateHeight(RendererContext &, float, float available_height) {
		return fixedHeight > 0? fixedHeight : available_height;
	}
}
