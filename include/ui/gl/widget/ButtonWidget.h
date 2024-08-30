#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

#include <glibmm/ustring.h>

namespace Game3 {
	class ButtonWidget: public Widget {
		public:
			ButtonWidget(float scale, float fixed_height, Color top_border_color, Color bottom_border_color, Color text_color, TexturePtr texture = getDefaultTexture());
			ButtonWidget(float scale, float fixed_height, Color border_color, Color text_color, TexturePtr texture = getDefaultTexture());
			ButtonWidget(float scale, float fixed_height, TexturePtr texture = getDefaultTexture());

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			void setText(Glib::ustring);
			void setFixedHeight(float);

		private:
			float fixedHeight{};
			Color topBorderColor;
			Color bottomBorderColor;
			Color textColor;
			Glib::ustring text;
			bool pressed = false;
			TexturePtr texture;

			float getTextScale(const RendererContext &, float height) const;

			static TexturePtr getDefaultTexture();
	};
}
