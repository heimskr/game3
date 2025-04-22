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
			void hide(TooltipUpdater &);

			/** Unconditionally enables the widget's visibility. */
			void show();

			/** Unconditionally enables the widget's visibility and sets the last updater. */
			template <typename U>
			void show(U &updater) {
				show();
				lastUpdater = updater.weak_from_this();
			}

			bool wasUpdatedBy(const TooltipUpdater &) const;

			void setText(UString);

			void setMaxWidth(float);

			void setBackgroundColor(const Color &);

			void setTextColor(const Color &);

			void setRegion(std::optional<Rectangle>);

			void setPositionOverride(std::optional<std::pair<float, float>>);

			template <typename U>
			bool setText(UString new_text, U &updater) {
				return updateField(std::move(new_text), &Tooltip::text, updater);
			}

			template <typename U>
			bool setMaxWidth(float new_max_width, U &updater) {
				return updateField(new_max_width, &Tooltip::maxWidth, updater);
			}

			template <typename U>
			bool setBackgroundColor(const Color &new_color, U &updater) {
				return updateField(new_color, &Tooltip::backgroundColor, updater);
			}

			template <typename U>
			bool setTextColor(const Color &new_color, U &updater) {
				return updateField(new_color, &Tooltip::textColor, updater);
			}

			template <typename U>
			bool setRegion(std::optional<Rectangle> new_region, U &updater) {
				return updateField(std::move(new_region), &Tooltip::region, updater);
			}

		private:
			UString text;
			float maxWidth{};
			bool visible = false;
			Color backgroundColor;
			Color textColor;
			std::weak_ptr<TooltipUpdater> lastUpdater;
			std::optional<Rectangle> region;
			/** Absolute (x, y) coordinates. */
			std::optional<std::pair<float, float>> positionOverride;

			float getTextScale() const;
			float getPadding() const;

			/** Returns whether the update took place. */
			template <typename T, typename U>
			bool updateField(T &&new_value, std::decay_t<T> Tooltip::*field, U &updater) {
				auto weak = updater.weak_from_this();
				if (lastUpdater.lock() == weak.lock())
					return false;

				lastUpdater = std::move(weak);
				this->*field = std::forward<T>(new_value);
				return true;
			}
	};
}
