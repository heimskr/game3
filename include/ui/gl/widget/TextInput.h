#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Autocompleter.h"
#include "ui/gl/HasFixedSize.h"

#include <sigc++/sigc++.h>

#include <functional>
#include <optional>
#include <vector>

namespace Game3 {
	class TextInput: public Widget, public HasFixedSize, public Autocompleter {
		public:
			sigc::signal<void(TextInput &, UIContext &)> onSubmit;
			sigc::signal<void(TextInput &, const UString &)> onChange;

			TextInput(float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness);
			TextInput(float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color);
			TextInput(float scale, float thickness);
			TextInput(float scale);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool keyPressed(UIContext &, uint32_t character, Modifiers) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;
			void onFocus(UIContext &) final;
			void onBlur(UIContext &) final;

			void setInteriorColor(Color);
			void setInteriorColor();

			void setSuggestions(std::optional<std::vector<UString>>);
			std::vector<UString> getRelevantSuggestions() const;

			const UString & getText() const;
			void setText(UIContext &, UString);
			void setText(UString);
			UString clear();
			void insert(UIContext &, gunichar);
			void eraseWord(UIContext &);
			void eraseCharacter(UIContext &);
			void eraseForward(UIContext &);
			void goLeft(UIContext &, std::size_t = 1);
			void goRight(UIContext &, std::size_t = 1);
			void goStart(UIContext &);
			void goEnd(UIContext &);

			void autocomplete(const UString &) final;

		private:
			float xOffset = 0;
			float thickness{};
			float cursorXOffset{};
			Color borderColor;
			Color interiorColor;
			Color textColor;
			Color cursorColor;
			UString text;
			UString::iterator cursorIterator = text.begin();
			std::size_t cursor = 0;
			bool cursorFixQueued = false;
			std::optional<std::vector<UString>> suggestions;
			std::optional<UString> deferredText;

			float getTextScale() const;
			float getXPadding() const;
			float getBoundary() const;
			float getCursorPosition() const;
			void adjustCursorOffset(float offset);
			void setCursorOffset(float);
			void fixCursorOffset();
			void changed(UIContext &);
			void forwardSuggestions(UIContext &);
	};
}
