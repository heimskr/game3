#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasAlignment.h"
#include "ui/gl/HasFixedSize.h"

namespace Game3 {
	class Button: public Widget, public HasFixedSize, public HasAlignment {
		public:
			Button(UIContext &, float scale, Color top_border_color, Color bottom_border_color, Color text_color, TexturePtr texture = getDefaultTexture());
			Button(UIContext &, float scale, Color border_color, Color text_color, TexturePtr texture = getDefaultTexture());
			Button(UIContext &, float scale, TexturePtr texture = getDefaultTexture());

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			bool click(int button, int x, int y, Modifiers) final;
			bool mouseDown(int button, int x, int y, Modifiers) final;
			bool mouseUp(int button, int x, int y, Modifiers) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			const UString & getText() const;
			void setText(UString);

		protected:
			virtual void renderLabel(const RendererContext &, const Rectangle &);
			virtual float getWidth(const RendererContext &, float height) const;

		private:
			bool pressed = false;
			Color topBorderColor;
			Color bottomBorderColor;
			Color textColor;
			Color topBorderColorPressed;
			Color bottomBorderColorPressed;
			Color textColorPressed;
			Color textureMultiplierPressed{1, 1, 1, 1};
			UString text;
			TexturePtr texture;

			float getTextScale(const RendererContext &, float height) const;
			void setColors(Color top, Color bottom, Color text_color);

			static TexturePtr getDefaultTexture();
			virtual float getMinimumPreferredHeight() const;
	};
}
