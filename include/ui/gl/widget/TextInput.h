#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Autocompleter.h"
#include "ui/gl/HasAlignment.h"
#include "ui/gl/HasFixedSize.h"

#include <sigc++/sigc++.h>

#include <chrono>
#include <functional>
#include <optional>
#include <vector>

namespace Game3 {
	class TextInput: public Widget, public HasFixedSize, public HasAlignment, public Autocompleter {
		public:
			sigc::signal<void(TextInput &, const UString &)> onSubmit;
			sigc::signal<void(TextInput &, const UString &)> onChange;
			sigc::signal<void(TextInput &, const UString &)> onAcceptSuggestion;
			std::function<bool(uint32_t, const UString::iterator &)> characterFilter;

			TextInput(UIContext &, float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness);
			TextInput(UIContext &, float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color);
			TextInput(UIContext &, float scale, float thickness);
			TextInput(UIContext &, float scale);

			void render(const RendererContext &, float x, float y, float width, float height) final;
			bool click(int button, int x, int y) final;
			bool keyPressed(uint32_t character, Modifiers, bool is_repeat) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;
			void onFocus() final;
			void onBlur() final;

			void setInteriorColor(Color);
			void setInteriorColor();

			void setTextColor(Color);
			void setTextColor();

			void setSuggestions(std::optional<std::vector<UString>>);
			std::vector<UString> getRelevantSuggestions() const;

			const UString & getText() const;
			void setText(UString);
			UString clear();
			void insert(uint32_t);
			void eraseWord();
			void eraseCharacter();
			void eraseForward();
			void goLeft(std::size_t = 1);
			void goRight(std::size_t = 1);
			void goStart();
			void goEnd();

			void autocomplete(const UString &) final;
			void hideDropdown() const;

		private:
			float xOffset = 0;
			float thickness{};
			float cursorXOffset{};
			Color borderColor;
			Color interiorColor;
			Color textColor;
			Color cursorColor;
			Color focusedCursorColor;
			UString text;
			UString::iterator cursorIterator = text.begin();
			std::size_t cursor = 0;
			bool cursorFixQueued = false;
			bool focused = false;
			std::optional<std::vector<UString>> suggestions;
			std::optional<UString> deferredText;
			std::optional<std::pair<uint32_t, std::chrono::system_clock::time_point>> lastPress;

			float getTextScale() const;
			float getXPadding() const;
			float getBoundary() const;
			float getCursorPosition() const;
			void adjustCursorOffset(float offset);
			void setCursorOffset(float);
			void fixCursorOffset();
			void changed();
			void forwardSuggestions();
			void makeDropdown();
			bool ownsDropdown() const;
	};
}
