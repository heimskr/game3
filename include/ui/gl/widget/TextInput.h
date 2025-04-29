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

			TextInput(UIContext &, float selfScale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness);
			TextInput(UIContext &, float selfScale, Color border_color, Color interior_color, Color text_color, Color cursor_color);
			TextInput(UIContext &, float selfScale, float thickness);
			TextInput(UIContext &, float selfScale);

			void render(const RendererContext &, float x, float y, float width, float height) final;
			bool click(int button, int x, int y, Modifiers) final;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) final;
			bool charPressed(uint32_t character, Modifiers) final;
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
			void goLeft(size_t = 1);
			void goRight(size_t = 1);
			void goStart();
			void goEnd();

			void setMultiline(bool);
			bool getMultiline() const { return multiline; }

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
			size_t lineNumber = 0;
			size_t columnNumber = 0;
			size_t textPosition = 0;
			bool cursorFixQueued = false;
			bool focused = false;
			bool multiline = false;
			std::optional<std::vector<UString>> suggestions;
			std::optional<UString> deferredText;
			std::optional<std::pair<uint32_t, std::chrono::system_clock::time_point>> lastPress;
			mutable std::optional<size_t> cachedLineCount;
			mutable std::optional<std::vector<size_t>> cachedColumnCounts;

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
			bool atBeginning() const;
			size_t getLineCount() const;
			size_t getLastLineNumber() const;
			size_t getColumnCount(size_t line) const;
			void setCachedColumnCounts() const;
			UStringSpan getLineSpan(size_t line_number) const;
	};
}
