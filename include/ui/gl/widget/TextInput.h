#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Autocompleter.h"
#include "ui/gl/HasAlignment.h"
#include "ui/gl/HasFixedSize.h"
#include "ui/gl/HasTooltipText.h"
#include "util/Gradual.h"

#include <sigc++/sigc++.h>

#include <chrono>
#include <functional>
#include <optional>
#include <vector>

namespace Game3 {
	class RectangleRenderer;
	class TextInput;
	class TextRenderer;

	class TextCursor {
		public:
			UString::iterator iterator;
			size_t lineNumber = 0;
			size_t columnNumber = 0;
			size_t position = 0;
			float xOffset = 0;
			bool primary = true;

			TextCursor(TextInput &, bool primary, UString::iterator);

			std::strong_ordering operator<=>(const TextCursor &) const;
			bool operator==(const TextCursor &) const;
			bool operator!=(const TextCursor &) const;

			void reset();
			void goLeft(size_t delta = 1);
			void goRight(size_t delta = 1);
			void goUp(size_t delta = 1);
			void goDown(size_t delta = 1);
			void goStart(bool within_line);
			void goEnd(bool within_line);
			float getXPosition(bool use_target = false) const;
			float getYPosition(bool use_target = false) const;
			bool atBeginning() const;
			bool atEnd() const;
			bool atLineBeginning() const;
			bool atLineEnd() const;

		private:
			TextInput *owner = nullptr;
	};

	class TextInput: public Widget, public HasFixedSize, public HasAlignment, public HasTooltipText, public Autocompleter {
		public:
			Color borderColor;
			Color interiorColor;
			Color textColor;
			Color cursorColor;
			Color focusedCursorColor;
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
			bool dragStart(int x, int y) final;
			bool dragUpdate(int x, int y) final;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) final;
			bool charPressed(uint32_t character, Modifiers) final;
			bool scroll(float x_delta, float y_delta, int x, int y, Modifiers) final;
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
			void insert(const UString &);
			void eraseWord();
			void eraseCharacter();
			void eraseForward();

			void setMultiline(bool);
			bool getMultiline() const { return multiline; }

			/** Returns whether there is an anchor at a different position from the cursor. */
			bool hasSelection() const;

			void copy();
			void paste();
			void cut();
			void selectAll();

			void autocomplete(const UString &) final;
			void hideDropdown() const;

		private:
			GradualFloat xOffset{0, 25};
			GradualFloat yOffset{0, 25};
			float thickness{};
			UString text;
			/** The start position of selected text. Note that this may be after the cursor if the user selects text backwards. */
			std::optional<TextCursor> anchor;
			/** The end position of selected text if any text is selected, or the position of the cursor otherwise. */
			std::optional<TextCursor> cursor;
			std::optional<std::vector<UString>> suggestions;
			std::optional<UString> deferredText;
			std::optional<std::pair<uint32_t, std::chrono::system_clock::time_point>> lastPress;
			mutable std::optional<std::vector<size_t>> cachedColumnCounts;
			mutable std::optional<size_t> cachedLineCount;
			mutable std::optional<size_t> widestLine;
			mutable std::optional<float> textWidth;
			mutable std::optional<float> textHeight;
			mutable std::optional<std::vector<std::optional<float>>> lineWidths;
			const TextCursor *offsetFixQueued = nullptr;
			bool focused = false;
			bool multiline = false;

			void goLeft(Modifiers);
			void goRight(Modifiers);
			void goUp(Modifiers);
			void goDown(Modifiers);
			void go(Modifiers, void (TextCursor::*)(size_t));
			float getTextScale() const;
			float getPadding() const;
			float getXBoundary() const;
			float getYBoundary() const;
			void fixXOffset(const TextCursor &);
			void fixYOffset(const TextCursor &);
			void changed();
			void forwardSuggestions();
			void makeDropdown();
			bool ownsDropdown() const;
			size_t & getLineCount() const;
			size_t getLastLineNumber() const;
			size_t getColumnCount(size_t line) const;
			void setCachedColumnCounts() const;
			UStringSpan getLineSpan(size_t line_number, size_t max_length = std::string::npos) const;
			float getCursorHeight() const;
			float getTextWidth() const;
			float getTextHeight() const;
			float getLineWidth(size_t line_number) const;
			std::pair<UString::iterator, UString::iterator> getIterators() const;
			std::pair<const TextCursor *, const TextCursor *> getCursors() const;
			TextRenderer & getTexter() const;
			bool ensureCursor();
			bool ensureAnchor();
			bool ensureCursor(std::optional<TextCursor> &);
			void clearCachedData() const;
			void renderSelection(RectangleRenderer &);
			std::pair<int, int> getLineAndColumn(int x, int y) const;
			void setAnchorAt(int x, int y);
			void home(Modifiers);
			void end(Modifiers);
			TextCursor seekLeft();
			TextCursor seekRight();

		friend class TextCursor;
	};
}
