#include "graphics/Color.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/ProgressBar.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	using namespace Game3;

	constexpr Color DEFAULT_INTERIOR_COLOR{"#ff0000"};
	constexpr Color DEFAULT_EXTERIOR_COLOR{0.6, 0.3, 0.0, 1.0};
	constexpr Color DEFAULT_BACKGROUND_COLOR{0.25, 0.0, 0.0, 1.0};
	constexpr std::chrono::milliseconds PROGRESS_UPDATE_TIME{100};
}

namespace Game3 {
	ProgressBar::ProgressBar(float scale, Color interior_color, Color background_color, Color exterior_color, float progress):
		Widget(scale),
		topInteriorColor(interior_color),
		bottomInteriorColor(topInteriorColor.darken()),
		backgroundColor(background_color),
		topExteriorColor(exterior_color),
		bottomExteriorColor(topExteriorColor.darken()),
		progress(progress) {}

	ProgressBar::ProgressBar(float scale, Color interior_color, float progress):
		ProgressBar(scale, interior_color, DEFAULT_BACKGROUND_COLOR, DEFAULT_EXTERIOR_COLOR, progress) {}

	ProgressBar::ProgressBar(float scale, float progress):
		ProgressBar(scale, DEFAULT_INTERIOR_COLOR, progress) {}

	void ProgressBar::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		fixSizes(width, height);
		Widget::render(ui, renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;
		std::shared_ptr<Tooltip> tooltip = ui.getTooltip();

		if (ui.checkMouseAbsolute(lastRectangle)) {
			if (progress != lastReportedProgress || !tooltip->wasUpdatedBy(*this)) {
				lastReportedProgress = progress;
				tooltip->setText(std::format("{:.1f}%", progress * 100));
				tooltip->setRegion(lastRectangle);
				tooltip->show(*this);
			}
		} else {
			tooltip->hide(*this);
			lastReportedProgress = -1;
		}

		float shown_progress = progress;

		if (progressUpdatePoint) {
			const auto time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - *progressUpdatePoint);
			const float lerp_progress = std::min(1.0, static_cast<double>(time_difference.count()) / static_cast<double>(PROGRESS_UPDATE_TIME.count()));

			if (lerp_progress == 1) {
				progressUpdatePoint.reset();
			} else if (oldProgress != progress) {
				shown_progress = lerp(oldProgress, progress, lerp_progress);
			}
		}

		const float bar_width = shown_progress * (width - 2 * scale);
		const float top_height = .6f * (height - 2 * scale);
		const float bottom_height = .4f * (height - 2 * scale);

		// Bar fill
		rectangler(topInteriorColor, x + scale, y + scale, bar_width, top_height);
		rectangler(bottomInteriorColor, x + scale, y + scale + top_height, bar_width, bottom_height);

		// Background
		rectangler(backgroundColor, x + scale + bar_width, y + scale, width - 2 * scale - bar_width, height - 2 * scale);

		// Top and bottom horizontal rectangles
		rectangler(topExteriorColor, x + scale, y, width - 2 * scale, scale);
		rectangler(bottomExteriorColor, x + scale, y + height - scale, width - 2 * scale, scale);

		// Corner rectangles
		rectangler(topExteriorColor, x + scale, y + scale, scale, scale);
		rectangler(topExteriorColor, x + width - scale * 2, y + scale, scale, scale);
		rectangler(bottomExteriorColor, x + scale, y + height - scale * 2, scale, scale);
		rectangler(bottomExteriorColor, x + width - scale * 2, y + height - scale * 2, scale, scale);

		// Side rectangles
		rectangler(topExteriorColor, x, y + scale, scale, top_height);
		rectangler(topExteriorColor, x + width - scale, y + scale, scale, top_height);
		rectangler(bottomExteriorColor, x, y + scale + top_height, scale, bottom_height);
		rectangler(bottomExteriorColor, x + width - scale, y + scale  + top_height, scale, bottom_height);
	}

	SizeRequestMode ProgressBar::getRequestMode() const {
		return SizeRequestMode::WidthForHeight;
	}

	void ProgressBar::measure(const RendererContext &, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (measure_orientation == Orientation::Horizontal) {
			minimum = natural = 0 < fixedWidth? fixedWidth : (horizontalExpand? for_width : getDefaultWidth());
		} else {
			minimum = natural = 0 < fixedHeight? fixedHeight : (verticalExpand? for_height : getDefaultHeight());
		}
	}

	void ProgressBar::setProgress(float new_progress) {
		if (std::abs(new_progress - progress) < 0.0001f)
			return;

		oldProgress = progress;
		progress = std::min(1.f, std::max(0.f, new_progress));
		progressUpdatePoint.emplace(std::chrono::system_clock::now());
	}

	float ProgressBar::getDefaultWidth() const {
		return 30 * scale;
	}

	float ProgressBar::getDefaultHeight() const {
		return 10 * scale;
	}
}