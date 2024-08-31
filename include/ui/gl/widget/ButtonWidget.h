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
			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			std::pair<float, float> calculateSize(const RendererContext &, float available_width, float available_height) final;

			const Glib::ustring & getText() const;
			float getFixedHeight() const;

			void setText(Glib::ustring);
			void setFixedHeight(float);

		protected:
			virtual void renderLabel(UIContext &, const RendererContext &, float width, float height);
			virtual void adjustWidth(const RendererContext &, float &width, float height) const;

		private:
			float fixedHeight{};
			bool pressed = false;
			Color topBorderColor;
			Color bottomBorderColor;
			Color textColor;
			Color topBorderColorPressed;
			Color bottomBorderColorPressed;
			Color textColorPressed;
			Color textureMultiplierPressed{1, 1, 1, 1};
			Glib::ustring text;
			TexturePtr texture;

			float getTextScale(const RendererContext &, float height) const;
			void setColors(Color top, Color bottom, Color text_color);

			static TexturePtr getDefaultTexture();
	};
}
