#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

#include <glibmm/ustring.h>

#include <optional>

namespace Game3 {
	class TooltipWidget: public Widget {
		public:
			TooltipWidget(float scale);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			/** Unconditionally disables the widget's visibility. */
			void hide();
			/** Hides the widget if the updater is the stored updater. */
			void hide(const Widget &updater);

			/** Unconditionally enables the widget's visibility. */
			void show();
			/** Unconditionally enables the widget's visibility and sets the last updater. */
			void show(const Widget &updater);

			bool wasUpdatedBy(const Widget &) const;

			void setText(Glib::ustring);
			bool setText(Glib::ustring, const Widget &updater);

			void setMaxWidth(float);
			bool setMaxWidth(float, const Widget &updater);

			void setBackgroundColor(const Color &);
			bool setBackgroundColor(const Color &, const Widget &updater);

			void setTextColor(const Color &);
			bool setTextColor(const Color &, const Widget &updater);

			void setRegion(std::optional<Rectangle>);
			bool setRegion(std::optional<Rectangle>, const Widget &updater);

		private:
			Glib::ustring text;
			float maxWidth{};
			bool visible = false;
			Color backgroundColor;
			Color textColor;
			std::weak_ptr<const Widget> lastUpdater;
			std::optional<Rectangle> region;

			float getTextScale() const;
			float getPadding() const;

			/** Returns whether the update took place. */
			template <typename T>
			bool updateField(T &&new_value, std::decay_t<T> TooltipWidget::*field, const Widget &updater) {
				auto weak = updater.weak_from_this();
				if (lastUpdater.lock() == weak.lock())
					return false;

				lastUpdater = std::move(weak);
				this->*field = std::forward<T>(new_value);
				return true;
			}
	};
}
