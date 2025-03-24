#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"

#include <memory>
#include <optional>
#include <utility>

namespace Game3 {
	class Tooltip: public Widget {
		public:
			Tooltip(UIContext &, float selfScale);

			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			/** Unconditionally disables the widget's visibility. */
			void hide();
			/** Hides the widget if the updater is the stored updater. */
			void hide(const Widget &updater);

			/** Unconditionally enables the widget's visibility. */
			void show();
			/** Unconditionally enables the widget's visibility and sets the last updater. */
			void show(const Widget &updater);

			bool wasUpdatedBy(const Widget &) const;

			void setText(UString);
			bool setText(UString, const Widget &updater);

			void setMaxWidth(float);
			bool setMaxWidth(float, const Widget &updater);

			void setBackgroundColor(const Color &);
			bool setBackgroundColor(const Color &, const Widget &updater);

			void setTextColor(const Color &);
			bool setTextColor(const Color &, const Widget &updater);

			void setRegion(std::optional<Rectangle>);
			bool setRegion(std::optional<Rectangle>, const Widget &updater);

			void setPositionOverride(std::optional<std::pair<float, float>>);

		private:
			UString text;
			float maxWidth{};
			bool visible = false;
			Color backgroundColor;
			Color textColor;
			std::weak_ptr<const Widget> lastUpdater;
			std::optional<Rectangle> region;
			/** Absolute (x, y) coordinates. */
			std::optional<std::pair<float, float>> positionOverride;

			float getTextScale() const;
			float getPadding() const;

			/** Returns whether the update took place. */
			template <typename T>
			bool updateField(T &&new_value, std::decay_t<T> Tooltip::*field, const Widget &updater) {
				auto weak = updater.weak_from_this();
				if (lastUpdater.lock() == weak.lock())
					return false;

				lastUpdater = std::move(weak);
				this->*field = std::forward<T>(new_value);
				return true;
			}
	};
}
