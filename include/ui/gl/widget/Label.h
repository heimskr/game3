#pragma once

#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasAlignment.h"
#include "ui/gl/HasFixedSize.h"
#include "ui/gl/HasTooltipText.h"

namespace Game3 {
	class Label: public Widget, public HasAlignment, public HasFixedSize, public HasTooltipText {
		public:
			Label(UIContext &, float selfScale, UString text = {}, Color = {0, 0, 0, 1});

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			void setText(UString);
			const UString & getText() const;

			void setTextColor(const Color &);
			const Color & getTextColor() const;

		protected:
			UString text;
			Color textColor;
			std::optional<UString> wrapped;
			float lastTextHeight = -1;
			float lastUnwrappedTextWidth = -1;

			float getTextScale() const;
			float getPadding() const;
			float getWrapWidth(float width) const;
			void tryWrap(const TextRenderer &, float width);
	};
}
