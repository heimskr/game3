#pragma once

#include "types/UString.h"
#include "ui/widget/Widget.h"
#include "ui/HasAlignment.h"
#include "ui/HasFixedSize.h"
#include "ui/HasTooltipText.h"

namespace Game3 {
	class Label: public Widget, public HasAlignment, public HasFixedSize, public HasTooltipText {
		public:
			Label(UIContext &, float selfScale, UString text = {}, Color textColor = {0, 0, 0, 1}, Color shadowColor = {0, 0, 0, 0}, Vector2d shadowOffset = {0, 0});

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			void setText(UString);
			const UString & getText() const;

			void setTextColor(const Color &);
			const Color & getTextColor() const;

			bool getMayWrap() const;
			void setMayWrap(bool);

		protected:
			UString text;
			Color textColor;
			Color shadowColor;
			Vector2d shadowOffset{0, 0};
			std::optional<UString> wrapped;
			float lastTextHeight = -1;
			float lastUnwrappedTextWidth = -1;
			bool mayWrap = true;

			float getTextScale() const;
			float getPadding() const;
			float getWrapWidth(float width) const;
			void tryWrap(const TextRenderer &, float width);
	};
}
