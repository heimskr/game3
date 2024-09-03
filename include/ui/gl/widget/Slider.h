#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasFixedSize.h"

#include <sigc++/sigc++.h>

#include <functional>
#include <optional>

namespace Game3 {
	class Slider: public Widget, public HasFixedSize {
		public:
			sigc::signal<void(Slider &, double)> onValueUpdate;

			Slider(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			bool dragStart(UIContext &, int x, int y) final;
			bool dragUpdate(UIContext &, int x, int y) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			const UString & getTooltipText();

			double getMinimum() const;
			double getMaximum() const;
			double getValue() const;
			double getStep() const;
			int getDisplayDigits() const;

			void setMinimum(double);
			void setMaximum(double);
			void setRange(double, double);
			void setValue(double);
			void setStep(double);
			void setDisplayDigits(int);

		private:
			double minimum{};
			double maximum{};
			double value{};
			double step{};
			/** Number of digits after the decimal point to display in the tooltip. */
			int displayDigits{};
			Color barColor;
			Color handleColor;
			std::optional<UString> tooltipText;

			float getBarHeight() const;
			float getHandleSize() const;
	};
}
