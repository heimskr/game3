#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

#include <glibmm/ustring.h>

#include <functional>

namespace Game3 {
	class TextInputWidget: public Widget {
		public:
			std::function<void(TextInputWidget &)> onSubmit;

			TextInputWidget(float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness);
			TextInputWidget(float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color);
			TextInputWidget(float scale, float thickness);
			TextInputWidget(float scale);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool keyPressed(UIContext &, uint32_t character, Modifiers) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			const Glib::ustring & getText() const;
			void setText(UIContext &, Glib::ustring);
			Glib::ustring clear();
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
			float thickness{};
			float cursorXOffset;
			Color borderColor;
			Color interiorColor;
			Color textColor;
			Color cursorColor;
			Glib::ustring text; // TODO: replace with non-Glib alternative
			Glib::ustring::iterator cursorIterator;
			size_t cursor = 0;
			bool cursorFixQueued = false;

			float getTextScale() const;
			float getXPadding() const;
			float getBoundary() const;
			float getCursorPosition() const;
			void adjustCursorOffset(float offset);
			void setCursorOffset(float);
			void fixCursorOffset();
	};
}
