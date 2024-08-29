#pragma once

#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

#include <glibmm/ustring.h>

namespace Game3 {
	class TextInputWidget: public Widget {
		public:
			TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, float thickness);
			TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color);
			TextInputWidget(float scale, float thickness);
			TextInputWidget(float scale);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool keyPressed(UIContext &, uint32_t character) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			void setText(Glib::ustring);

		private:
			float xOffset = 0;
			float scale{};
			float thickness{};
			Color exteriorColor;
			Color interiorColor;
			Color textColor;
			Glib::ustring text; // TODO: replace with non-Glib alternative
	};
}
