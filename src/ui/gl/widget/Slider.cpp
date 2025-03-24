#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Slider.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/UIContext.h"

namespace {
	using namespace Game3;

	constexpr Color DEFAULT_BAR_COLOR{"#cc9a59"};
	constexpr Color DEFAULT_HANDLE_COLOR{"#604620"};
}

namespace Game3 {
	Slider::Slider(UIContext &ui, float selfScale):
		Widget(ui, selfScale),
		barColor(DEFAULT_BAR_COLOR),
		handleColor(DEFAULT_HANDLE_COLOR) {}

	void Slider::render(const RendererContext &renderers, float x, float y, float width, float height) {
		fixSizes(width, height, ui.scale);

		Widget::render(renderers, x, y, width, height);

		const float handle_size = getHandleSize();
		x += handle_size / 2;
		width -= handle_size;

		if (width <= 0 || height <= 0) {
			return;
		}

		RectangleRenderer &rectangler = renderers.rectangle;
		const float bar_height = getBarHeight();
		const float x_pos = minimum == maximum? 0 : width * (value - minimum) / (maximum - minimum);

		rectangler.drawOnScreen(barColor, x, y + (height - bar_height) / 2, width, bar_height);

		Rectangle handle_rectangle(x + x_pos - handle_size / 2, y + (height - handle_size) / 2, handle_size, handle_size);
		rectangler.drawOnScreen(handleColor, handle_rectangle);

		std::shared_ptr<Tooltip> tooltip = ui.getTooltip();

		if (isDragging() || (!ui.anyDragUpdaters() && ui.checkMouse(lastRectangle))) {
			tooltip->setText(getTooltipText());
			tooltip->setRegion(std::nullopt);
			handle_rectangle = ui.scissorStack.getTop().rectangle + handle_rectangle;
			tooltip->setPositionOverride(std::pair<float, float>{handle_rectangle.x + handle_size, handle_rectangle.y + handle_size});
			tooltip->show(*this);
		} else {
			tooltip->hide(*this);
		}
	}

	bool Slider::dragStart(int x, int y) {
		Widget::dragStart(x, y);
		ui.addDragUpdater(shared_from_this());
		return true;
	}

	bool Slider::dragUpdate(int x, int y) {
		Widget::dragUpdate(x, y);

		if (!isDragging()) {
			return false;
		}

		const float difference = std::min(lastRectangle.width, x - lastRectangle.x);

		if (difference < 0) {
			setValue(minimum);
			return true;
		}

		if (minimum == maximum) {
			setValue(maximum);
			return true;
		}

		setValue(minimum + difference / lastRectangle.width * (maximum - minimum));
		return true;
	}

	bool Slider::dragEnd(int x, int y) {
		Widget::dragEnd(x, y);
		onRelease(*this, value);
		return true;
	}

	SizeRequestMode Slider::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Slider::measure(const RendererContext &, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			if (0 < fixedWidth) {
				minimum = natural = fixedWidth;
			} else {
				minimum = 0;
				natural = for_width;
			}
		} else {
			if (0 < fixedHeight) {
				minimum = natural = fixedHeight;
			} else {
				minimum = 0;
				natural = for_height;
			}
		}
	}

	const UString & Slider::getTooltipText() {
		if (!tooltipText) {
			UString &text = tooltipText.emplace(std::format("{:f}", value));
			if (0 <= displayDigits) {
				if (const std::size_t period = text.find('.'); period != UString::npos) {
					if (displayDigits == 0) {
						text.erase(period);
						if (!text.empty() && text[0] == '-') {
							text.erase(0, 1);
						}
					} else if (period + 1 + displayDigits < text.length()) {
						text.erase(period + 1 + displayDigits);
					}
				}
			}
		}

		return *tooltipText;
	}

	double Slider::getMinimum() const {
		return minimum;
	}

	double Slider::getMaximum() const {
		return maximum;
	}

	double Slider::getValue() const {
		return value;
	}

	double Slider::getStep() const {
		return step;
	}

	int Slider::getDisplayDigits() const {
		return displayDigits;
	}

	void Slider::setMinimum(double new_minimum) {
		minimum = new_minimum;
	}

	void Slider::setMaximum(double new_maximum) {
		maximum = new_maximum;
	}

	void Slider::setRange(double new_minimum, double new_maximum) {
		setMinimum(new_minimum);
		setMaximum(new_maximum);
	}

	void Slider::setValue(double new_value) {
		if (step > 0) {
			new_value -= std::remainder(new_value - minimum, step);
		}

		if (value != new_value) {
			tooltipText.reset();
			value = new_value;
			onValueUpdate(*this, value);
		}
	}

	void Slider::setStep(double new_step) {
		step = new_step;
	}

	void Slider::setDisplayDigits(int new_display_digits) {
		displayDigits = new_display_digits;
	}

	float Slider::getBarHeight() const {
		return selfScale * 2;
	}

	float Slider::getHandleSize() const {
		return selfScale * 4;
	}
}
