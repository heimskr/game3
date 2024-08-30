#pragma once

#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

#include <glibmm/ustring.h>

namespace Game3 {
	class TextInputWidget: public Widget {
		public:
			TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, Color cursor_color, float thickness);
			TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, Color cursor_color);
			TextInputWidget(float scale, float thickness);
			TextInputWidget(float scale);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool keyPressed(UIContext &, uint32_t character, Modifiers) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			void setText(Glib::ustring);
			void insert(UIContext &, gunichar);
			void eraseWord(UIContext &);
			void eraseCharacter(UIContext &);
			void eraseForward(UIContext &);
			void goLeft(UIContext &, size_t = 1);
			void goRight(UIContext &, size_t = 1);
			void goStart(UIContext &);
			void goEnd(UIContext &);

		private:
			float xOffset = 0;
			float scale{};
			float thickness{};
			float cursorXOffset;
			Color exteriorColor;
			Color interiorColor;
			Color textColor;
			Color cursorColor;
			Glib::ustring text; // TODO: replace with non-Glib alternative
			Glib::ustring::iterator cursorIterator;
			size_t cursor = 0;

			float getTextScale() const;
	};
}
