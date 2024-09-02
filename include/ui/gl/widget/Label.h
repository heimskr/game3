#pragma once

#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasAlignment.h"
#include "ui/gl/HasFixedSize.h"

namespace Game3 {
	class Label: public Widget, public HasAlignment, public HasFixedSize {
		public:
			Label(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setText(UIContext &, UString);
			const UString & getText() const;

		protected:
			UString text;
			std::optional<UString> wrapped;
			float lastTextHeight = -1;

			float getTextScale() const;
			float getPadding() const;
			float getWrapWidth(float width) const;
			void tryWrap(const TextRenderer &, float width);
	};
}
