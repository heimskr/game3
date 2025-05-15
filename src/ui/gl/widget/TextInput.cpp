#include "math/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/gl/Util.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/Window.h"
#include "util/Util.h"

#include "clip.h"

namespace {
	constexpr Game3::Color DEFAULT_BORDER_COLOR{"#926641"};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{"#341903"};
	constexpr Game3::Color DEFAULT_CURSOR_COLOR{"#927a66"};
	constexpr float DEFAULT_THICKNESS = 1;
	constexpr std::chrono::milliseconds KEY_REPEAT_DELAY{500};

	bool isStopChar(Game3::UString::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max()) {
			return false;
		}

		static const std::string_view stops = "_.,/-=+ \t\n";
		return stops.find(static_cast<char>(unicharacter)) != std::string_view::npos;
	}

	bool isWhitespace(Game3::UString::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max()) {
			return false;
		}

		static const std::string_view stops = " \t\n";
		return stops.find(static_cast<char>(unicharacter)) != std::string_view::npos;
	}
}

namespace Game3 {
	TextCursor::TextCursor(TextInput &owner, bool primary, UString::iterator iterator):
		iterator(iterator),
		primary(primary),
		owner(owner) {}

	std::strong_ordering TextCursor::operator<=>(const TextCursor &other) const {
		if (this == &other) {
			return std::strong_ordering::equivalent;
		}

		assert(&owner == &other.owner);
		return position <=> other.position;
	}

	bool TextCursor::operator==(const TextCursor &other) const {
		return this == &other || (&owner == &other.owner && position == other.position);
	}

	bool TextCursor::operator!=(const TextCursor &other) const {
		return this != &other && (&owner != &other.owner || position != other.position);
	}

	void TextCursor::reset() {
		iterator = owner.text.begin();
		lineNumber = 0;
		columnNumber = 0;
		position = 0;
		xOffset = 0;
	}

	void TextCursor::goLeft(size_t delta) {
		if (position == 0 || delta == 0) {
			return;
		}

		if (position <= delta) {
			reset();
			owner.xOffset = 0;
			owner.yOffset = 0;
			return;
		}

		position -= delta;
		std::advance(iterator, -static_cast<ssize_t>(delta));

		while (delta != 0) {
			if (columnNumber >= delta) {
				columnNumber -= delta;
				break;
			}

			assert(lineNumber != 0);
			delta -= columnNumber + 1;
			columnNumber = owner.getColumnCount(--lineNumber);
		}

		if (columnNumber == 0) {
			xOffset = 0;
		} else {
			xOffset = owner.getTexter().textWidth(owner.getLineSpan(lineNumber, columnNumber));
		}

		owner.fixXOffset(*this);
		owner.fixYOffset(*this);
	}

	void TextCursor::goRight(size_t delta) {
		const size_t text_size = owner.text.size();

		if (text_size == 0) {
			reset();
			owner.xOffset = 0;
			owner.yOffset = 0;
			return;
		}

		if (position == text_size || delta == 0) {
			owner.fixXOffset(*this);
			owner.fixYOffset(*this);
			return;
		}

		delta = std::min(delta, text_size - position);

		position += delta;
		std::advance(iterator, delta);

		while (delta != 0) {
			const size_t line_length = owner.getColumnCount(lineNumber);

			if (columnNumber + delta <= line_length) {
				columnNumber += delta;
				break;
			}

			delta -= line_length - columnNumber + 1;
			columnNumber = 0;
			++lineNumber;
			assert(lineNumber < owner.getLineCount());
		}

		if (columnNumber == 0) {
			xOffset = 0;
		} else {
			xOffset = owner.getTexter().textWidth(owner.getLineSpan(lineNumber, columnNumber));
		}

		owner.fixXOffset(*this);
		owner.fixYOffset(*this);
	}

	void TextCursor::goStart(bool within_line) {
		if (within_line) {
			position -= columnNumber;
			while (columnNumber > 0) {
				--columnNumber;
				--iterator;
			}
		} else {
			lineNumber = 0;
			columnNumber = 0;
			iterator = owner.text.begin();
			position = 0;
		}

		xOffset = 0;
		owner.xOffset = 0;
	}

	void TextCursor::goEnd(bool within_line) {
		if (within_line) {
			const size_t column_count = owner.getColumnCount(lineNumber);
			position += column_count - columnNumber;
			while (columnNumber < column_count) {
				++columnNumber;
				++iterator;
			}
		} else {
			lineNumber = owner.getLastLineNumber();
			columnNumber = owner.getColumnCount(lineNumber);
			iterator = owner.text.end();
			position = owner.text.size();
		}

		xOffset = owner.getTexter().textWidth(owner.getLineSpan(lineNumber));
		owner.fixXOffset(*this);
	}

	void TextCursor::goUp(size_t delta) {
		for (size_t i = 0; i < delta; ++i) {
			if (lineNumber == 0) {
				if (!atBeginning()) {
					goStart(false);
				}
			} else {
				while (0 < position) {
					--position;
					if (*--iterator == '\n') {
						break;
					}
				}

				const size_t old_column_number = std::exchange(columnNumber, owner.getColumnCount(--lineNumber));

				while (columnNumber > old_column_number) {
					--columnNumber;
					--iterator;
					--position;
				}

				auto last = iterator;
				auto start = std::prev(last, columnNumber);

				xOffset = owner.getTexter().textWidth(UStringSpan(start, last));
				owner.fixXOffset(*this);
				owner.fixYOffset(*this);
			}
		}
	}

	void TextCursor::goDown(size_t delta) {
		bool fix = false;

		for (size_t i = 0; i < delta; ++i) {
			if (lineNumber + 1 >= owner.getLineCount()) {
				if (!atEnd()) {
					goEnd(false);
				}
			} else {
				const size_t text_length = owner.text.length();

				while (position < text_length) {
					++position;
					if (*iterator++ == '\n') {
						break;
					}
				}

				auto start = iterator;
				auto last = start;

				size_t i = 0;

				for (; i < columnNumber && position < text_length; ++i) {
					++position;
					if (*iterator++ == '\n') {
						break;
					}
					++last;
				}

				columnNumber = i;
				++lineNumber;
				TextRenderer &texter = owner.getTexter();
				xOffset = texter.textWidth(UStringSpan(start, last));
				fix = true;
			}
		}

		if (fix) {
			owner.fixXOffset(*this);
			owner.fixYOffset(*this);
		}
	}

	float TextCursor::getXPosition(bool use_target) const {
		return owner.getPadding() - (use_target? owner.xOffset.getTarget() : owner.xOffset) * owner.getScale() + xOffset * owner.getTextScale();
	}

	float TextCursor::getYPosition(bool use_target) const {
		return owner.getPadding() - (use_target? owner.yOffset.getTarget() : owner.yOffset) * owner.getScale() + lineNumber * owner.getCursorHeight();
	}

	bool TextCursor::atBeginning() const {
		return lineNumber == 0 && columnNumber == 0;
	}

	bool TextCursor::atEnd() const {
		return iterator == owner.text.end();
	}

	bool TextCursor::atLineBeginning() const {
		return columnNumber == 0;
	}

	bool TextCursor::atLineEnd() const {
		return columnNumber == owner.getColumnCount(lineNumber);
	}

	TextInput::TextInput(UIContext &ui, float selfScale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness):
		Widget(ui, selfScale),
		HasFixedSize(-1, selfScale * TEXT_INPUT_HEIGHT_FACTOR),
		thickness(thickness),
		borderColor(border_color),
		interiorColor(interior_color),
		textColor(text_color),
		cursorColor(cursor_color),
		focusedCursorColor(cursorColor.darken(3)),
		cursor(std::in_place, *this, true, text.begin()) {}

	TextInput::TextInput(UIContext &ui, float selfScale, Color border_color, Color interior_color, Color text_color, Color cursor_color):
		TextInput(ui, selfScale, border_color, interior_color, text_color, cursor_color, DEFAULT_THICKNESS) {}

	TextInput::TextInput(UIContext &ui, float selfScale, float thickness):
		TextInput(ui, selfScale, DEFAULT_BORDER_COLOR, DEFAULT_TEXTINPUT_INTERIOR_COLOR, DEFAULT_TEXT_COLOR, DEFAULT_CURSOR_COLOR, thickness) {}

	TextInput::TextInput(UIContext &ui, float selfScale):
		TextInput(ui, selfScale, DEFAULT_THICKNESS) {}

	void TextInput::render(const RendererContext &renderers, float x, float y, float width, float height) {
		const float scale = getScale();

		xOffset.setMultiplier(scale);
		yOffset.setMultiplier(scale);
		xOffset.tick(renderers.delta);
		yOffset.tick(renderers.delta);

		if (width < -1 || height < -1) {
			Widget::render(renderers, x, y, width, height);
			return;
		}

		const float original_height = height;

		if (0 < fixedHeight) {
			height = fixedHeight * ui.scale;
		}

		adjustCoordinate(Orientation::Horizontal, x, width, width);
		adjustCoordinate(Orientation::Vertical, y, original_height, height);

		Widget::render(renderers, x, y, width, height);

		if (shouldCull()) {
			return;
		}

		if (const TextCursor *target = std::exchange(offsetFixQueued, nullptr)) {
			fixXOffset(*target);
			fixYOffset(*target);
		}

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const float start = thickness * scale;
		// TODO: check for negative sizes
		auto interior = Rectangle(x + start, y + start, width - 2 * start, height - 2 * start);

		rectangler(borderColor, Rectangle(x, y, width, height * 0.6));
		rectangler(borderColor.darken(), Rectangle(x, y + height * 0.6, width, height * 0.4));
		rectangler(interiorColor, interior);

		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		if (cursor) {
			if (!anchor || *anchor == *cursor) {
				Color color = focused? focusedCursorColor : cursorColor;
				const float cursor_height = getCursorHeight();
				const float pixel = start / 2;
				rectangler(color, cursor->getXPosition(), multiline? cursor->getYPosition() : start, pixel, cursor_height);
			} else {
				renderSelection(rectangler);
			}
		}

		texter(text, TextRenderOptions{
			.x = start - xOffset * scale,
			.y = 2 * start - yOffset * scale,
			.scaleX = getTextScale(),
			.scaleY = getTextScale(),
			.color = textColor,
			.alignTop = true,
			.shadow{0, 0, 0, 0},
			.ignoreNewline = !multiline,
		});
	}

	bool TextInput::click(int button, int x, int y, Modifiers modifiers) {
		if (Widget::click(button, x, y, modifiers)) {
			return true;
		}

		if (button == LEFT_BUTTON) {
			ui.focusWidget(shared_from_this());
			setAnchorAt(x - lastRectangle.x, y - lastRectangle.y);
			anchor.reset();
			return true;
		}

		return false;
	}

	bool TextInput::dragStart(int x, int y) {
		if (Widget::dragStart(x, y)) {
			return true;
		}

		WidgetPtr self = getSelf();
		ui.focusWidget(self);
		ui.addDragUpdater(std::move(self));
		setAnchorAt(x - lastRectangle.x, y - lastRectangle.y);

		return true;
	}

	bool TextInput::dragUpdate(int x, int y) {
		if (Widget::dragUpdate(x, y)) {
			return true;
		}

		const auto [line_number, column] = getLineAndColumn(x - lastRectangle.x, y - lastRectangle.y);
		if (!ensureCursor()) {
			cursor->reset();
		}
		cursor->goDown(line_number);
		cursor->goRight(column);

		return true;
	}

	bool TextInput::keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) {
		if (!is_repeat) {
			lastPress = {key, getTime()};
		} else if (lastPress && ((lastPress->first == key && getTime() - lastPress->second < KEY_REPEAT_DELAY) || (lastPress->first != key))) {
			if (lastPress->first != key && !isModifierKey(key)) {
				lastPress = {key, getTime()};
			}

			return true;
		}

		if (modifiers.onlyCtrl()) {
			switch (key) {
				case GLFW_KEY_BACKSPACE:
					eraseWord();
					changed();
					break;

				default:
					break;
			}

			// Ignore other ctrl sequences.
			return true;
		}

		switch (key) {
			case GLFW_KEY_BACKSPACE:
				eraseCharacter();
				changed();
				return true;

			case GLFW_KEY_DELETE:
				eraseForward();
				changed();
				return true;

			case GLFW_KEY_LEFT:
				goLeft(modifiers);
				return true;

			case GLFW_KEY_RIGHT:
				goRight(modifiers);
				return true;

			case GLFW_KEY_HOME:
				if (cursor) {
					cursor->goStart(!modifiers.ctrl);
					xOffset = 0;
				}
				return true;

			case GLFW_KEY_END:
				if (cursor) {
					cursor->goEnd(!modifiers.ctrl);
				}
				return true;

			case GLFW_KEY_UP:
				goUp(modifiers);
				return true;

			case GLFW_KEY_DOWN:
				goDown(modifiers);
				return true;

			case GLFW_KEY_MENU:
				return true;

			case GLFW_KEY_ESCAPE:
				if (ownsDropdown()) {
					hideDropdown();
				} else {
					ui.unfocusWidget(shared_from_this());
				}
				return true;

			case GLFW_KEY_ENTER:
			case GLFW_KEY_KP_ENTER:
				if (!multiline || modifiers.onlyShift()) {
					onSubmit(*this, text);
				} else {
					charPressed('\n', modifiers);
				}
				return true;

			default:
				break;
		}

		return true;
	}

	bool TextInput::charPressed(uint32_t codepoint, Modifiers modifiers) {
		if (modifiers.onlyCtrl()) {
			switch (codepoint) {
				case 'c':
					copy();
					break;

				case 'v':
					paste();
					break;

				case 'x':
					cut();
					break;

				case 'a':
					selectAll();
					break;

				default:
					break;
			}

			// Ignore other ctrl sequences.
			return true;
		}

		insert(static_cast<gunichar>(codepoint));
		changed();
		return true;
	}

	bool TextInput::scroll(float x_delta, float y_delta, int x, int y, Modifiers modifiers) {
		constexpr float scroll_speed = 2;

		if (multiline && y_delta != 0) {
			const float height = (getTextHeight() - lastRectangle.height) / getScale();
			if (height >= 0 || yOffset != 0) {
				if (y_delta < 0) {
					if (height >= 0) {
						yOffset.setValue(std::min<float>(height, yOffset - scroll_speed * y_delta));
					}
					return true;
				}

				const float new_offset = std::max<float>(0, yOffset - scroll_speed * y_delta);
				if (yOffset != new_offset || yOffset == 0) {
					yOffset.setValue(new_offset);
					return true;
				}
			}
		}

		if (x_delta != 0) {
			const float width = (getTextWidth() - lastRectangle.width) / getScale() + 4 * thickness;
			if (width >= 0 || xOffset != 0) {
				if (x_delta < 0) {
					if (width >= 0) {
						xOffset.setValue(std::min<float>(width, xOffset - scroll_speed * x_delta));
					}
					return true;
				}

				const float new_offset = std::max<float>(0, xOffset - scroll_speed * x_delta);
				if (xOffset != new_offset || xOffset == 0) {
					xOffset.setValue(new_offset);
					return true;
				}
			}
		}

		return Widget::scroll(x_delta, y_delta, x, y, modifiers);
	}

	SizeRequestMode TextInput::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void TextInput::measure(const RendererContext &renderers, Orientation orientation, float for_width, float, float &minimum, float &natural) {
		const float border = 2 * thickness * selfScale;

		if (orientation == Orientation::Horizontal) {
			if (0 < fixedWidth) {
				minimum = natural = fixedWidth * ui.scale;
			} else {
				minimum = border;
				natural = std::min(for_width, border + renderers.text.textWidth(text, getTextScale()));
			}
		} else {
			if (0 < fixedHeight) {
				minimum = natural = fixedHeight * ui.scale;
			} else {
				minimum = border;
				natural = border + renderers.text.textHeight(text, getTextScale(), multiline? -1 : for_width - border) + getCursorHeight();
			}
		}
	}

	void TextInput::onFocus() {
		focused = true;
		makeDropdown();
	}

	void TextInput::onBlur() {
		focused = false;

		std::shared_ptr<AutocompleteDropdown> dropdown = ui.getAutocompleteDropdown();
		if (dropdown && dropdown->checkParent(*this)) {
			ui.setAutocompleteDropdown(nullptr);
		}
	}

	void TextInput::setInteriorColor(Color color) {
		interiorColor = color;
	}

	void TextInput::setInteriorColor() {
		interiorColor = DEFAULT_TEXTINPUT_INTERIOR_COLOR;
	}

	void TextInput::setTextColor(Color color) {
		textColor = color;
	}

	void TextInput::setTextColor() {
		textColor = DEFAULT_TEXT_COLOR;
	}

	void TextInput::setSuggestions(std::optional<std::vector<UString>> new_suggestions) {
		suggestions = std::move(new_suggestions);
	}

	std::vector<UString> TextInput::getRelevantSuggestions() const {
		std::vector<UString> relevant;

		if (suggestions) {
			std::string_view raw = text.raw();
			for (const UString &suggestion: *suggestions) {
				if (suggestion.length() > text.length() && suggestion.raw().starts_with(raw)) {
					relevant.push_back(suggestion);
				}
			}
		}

		return relevant;
	}

	const UString & TextInput::getText() const {
		return text;
	}

	void TextInput::setText(UString new_text) {
		if (text != new_text) {
			text = std::move(new_text);
			onChange(*this, text);
		}

		clearCachedData();
		anchor.reset();
		cursor.emplace(*this, true, text.begin()).goEnd(false);
	}

	UString TextInput::clear() {
		UString out = std::move(text);
		cursor.reset();
		anchor.reset();
		xOffset = 0;
		yOffset = 0;
		return out;
	}

	void TextInput::insert(uint32_t character) {
		ensureCursor();

		if (characterFilter && !characterFilter(character, cursor->iterator)) {
			return;
		}

		if (anchor) {
			if (anchor->position != cursor->position) {
				auto [begin, end] = getIterators();
				text.replace(begin, end, 1, character);
			} else {
				cursor->iterator = std::next(text.insert(cursor->iterator, character));
			}
			anchor.reset();
		} else {
			cursor->iterator = std::next(text.insert(cursor->iterator, character));
		}

		++cursor->position;
		if (character == '\n') {
			++cursor->lineNumber;
			cursor->columnNumber = 0;
			cursor->xOffset = 0;
			xOffset = 0;
			++getLineCount();
			cachedColumnCounts.reset();
			fixYOffset(*cursor);
			textHeight.reset();
			textWidth.reset();
			widestLine.reset();
		} else {
			if (cachedColumnCounts) {
				++cachedColumnCounts->at(cursor->lineNumber);
			}
			++cursor->columnNumber;
			const float width = getTexter().textWidth(character);
			cursor->xOffset += width;
			if (widestLine == cursor->lineNumber) {
				textWidth.reset();
			}
			fixXOffset(*cursor);
		}
	}

	void TextInput::insert(const UString &string) {
		ensureCursor();

		const auto [left, right] = getCursors();
		const size_t new_position = left->position + string.size();

		if (hasSelection()) {
			text.replace(left->iterator, right->iterator, string);
		} else {
			text.insert(left->position, string);
		}

		clearCachedData();
		cursor->reset();
		cursor->goRight(new_position);
		anchor.reset();
		fixXOffset(*cursor);
		fixYOffset(*cursor);
	}

	void TextInput::eraseWord() {
		ensureCursor();

		if (hasSelection()) {
			eraseCharacter();
			return;
		}

		if (cursor->atBeginning()) {
			return;
		}

		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.

		if (isWhitespace(cursor->iterator)) {
			do {
				eraseCharacter();
			} while (cursor->iterator != text.begin() && isWhitespace(cursor->iterator));

			while (cursor->iterator != text.begin() && !isStopChar(cursor->iterator)) {
				eraseCharacter();
			}

			return;
		}

		if (isStopChar(cursor->iterator)) {
			do {
				eraseCharacter();
			} while (cursor->iterator != text.begin() && isStopChar(cursor->iterator));

			return;
		}

		do {
			eraseCharacter();
		} while (cursor->iterator != text.begin() && !isStopChar(cursor->iterator));

		while (cursor->iterator != text.begin() && isWhitespace(cursor->iterator)) {
			eraseCharacter();
		}
	}

	void TextInput::eraseCharacter() {
		ensureCursor();

		if (hasSelection()) {
			const auto [left, right] = getCursors();
			auto iterator = text.erase(left->iterator, right->iterator);

			// Do compiler-generated implementations of T & T::operator=(const T &)
			// do an address check and return early if the addresses are equal?
			// If not, I might need to implement one myself. Otherwise this might be silly.
			cursor.emplace(*left);

			cursor->iterator = iterator;
			anchor.reset();
			return;
		}

		if (cursor->atBeginning()) {
			return;
		}

		TextRenderer &texter = getTexter();

		if (*--cursor->iterator == '\n') {
			cursor->columnNumber = getColumnCount(--cursor->lineNumber);
			--getLineCount();
			cachedColumnCounts.reset();
			fixYOffset(*cursor);
			cursor->xOffset = texter.textWidth(getLineSpan(cursor->lineNumber));
			textWidth.reset();
			textHeight.reset();
			widestLine.reset();
		} else {
			if (cachedColumnCounts) {
				--cachedColumnCounts->at(cursor->lineNumber);
			}
			--cursor->columnNumber;
			cursor->xOffset -= texter.textWidth(*cursor->iterator);
			if (widestLine == cursor->lineNumber) {
				textWidth.reset();
				widestLine.reset();
			}
		}

		fixXOffset(*cursor);
		--cursor->position;
		cursor->iterator = text.erase(cursor->iterator);
	}

	void TextInput::eraseForward() {
		ensureCursor();

		if (hasSelection()) {
			eraseCharacter();
			return;
		}

		if (!text.empty() && cursor->iterator != text.end()) {
			if (*cursor->iterator == '\n') {
				--getLineCount();
				cachedColumnCounts.reset();
				textHeight.reset();
				textWidth.reset();
				widestLine.reset();
			} else {
				if (cachedColumnCounts) {
					--cachedColumnCounts->at(cursor->lineNumber);
				}

				if (widestLine == cursor->lineNumber) {
					textWidth.reset();
					widestLine.reset();
				}
			}
			cursor->iterator = text.erase(cursor->iterator);
		}
	}

	void TextInput::autocomplete(const UString &completion) {
		setText(completion);
		onAcceptSuggestion(*this, completion);
	}

	void TextInput::goLeft(Modifiers modifiers) {
		if (!modifiers.onlyShift() && hasSelection()) {
			auto [left, right] = getCursors();
			cursor.emplace(*left);
			anchor.reset();
			return;
		}

		go(modifiers, &TextCursor::goLeft);
	}

	void TextInput::goRight(Modifiers modifiers) {
		if (!modifiers.onlyShift() && hasSelection()) {
			auto [left, right] = getCursors();
			cursor.emplace(*right);
			anchor.reset();
			return;
		}

		go(modifiers, &TextCursor::goRight);
	}

	void TextInput::goUp(Modifiers modifiers) {
		go(modifiers, &TextCursor::goUp);
	}

	void TextInput::goDown(Modifiers modifiers) {
		go(modifiers, &TextCursor::goDown);
	}

	void TextInput::go(Modifiers modifiers, void (TextCursor::*function)(size_t)) {
		ensureCursor();

		if (modifiers.onlyShift()) {
			if (!anchor) {
				anchor.emplace(*cursor);
			}
			((*cursor).*function)(1);
		} else {
			anchor.reset();
			((*cursor).*function)(1);
		}
	}

	float TextInput::getTextScale() const {
		return getScale() / 16;
	}

	float TextInput::getPadding() const {
		return thickness * getScale();
	}

	float TextInput::getXBoundary() const {
		return lastRectangle.width - getPadding();
	}

	float TextInput::getYBoundary() const {
		return lastRectangle.height - getPadding();
	}

	void TextInput::fixXOffset(const TextCursor &target) {
		if (lastRectangle.width < 0) {
			offsetFixQueued = &target;
			return;
		}

		const float visual = target.getXPosition(true);
		const float boundary = getXBoundary();
		const float padding = getPadding();

		if (visual > boundary) {
			xOffset += (visual - boundary + padding * 2) / getScale();
		} else if (visual < padding) {
			xOffset = 0;
		}
	}

	void TextInput::fixYOffset(const TextCursor &target) {
		if (lastRectangle.height < 0) {
			offsetFixQueued = &target;
			return;
		}

		const float visual = target.getYPosition(true);
		const float boundary = getYBoundary();
		const float padding = getPadding();
		const float cursor_height = getCursorHeight();

		if (visual + cursor_height > boundary) {
			yOffset += (visual - boundary + padding * 2 + cursor_height) / getScale();
		} else if (visual < padding) {
			yOffset = 0;
		}
	}

	void TextInput::changed() {
		onChange(*this, text);
		forwardSuggestions();
	}

	void TextInput::forwardSuggestions() {
		std::shared_ptr<AutocompleteDropdown> dropdown = ui.getAutocompleteDropdown();

		if (dropdown && dropdown->checkParent(*this)) {
			dropdown->setSuggestions(getRelevantSuggestions());
			// dropdown->queueConstrainSize();
		} else {
			makeDropdown();
		}
	}

	void TextInput::makeDropdown() {
		std::vector<UString> relevant = getRelevantSuggestions();
		if (relevant.empty()) {
			return;
		}

		auto dropdown = std::make_shared<AutocompleteDropdown>(ui, selfScale);
		dropdown->init();
		dropdown->setParent(std::dynamic_pointer_cast<Autocompleter>(shared_from_this()));
		dropdown->setOrigin({lastRectangle.x, lastRectangle.y + lastRectangle.height});
		dropdown->setSuggestions(std::move(relevant));
		dropdown->setFixedSize(lastRectangle.width, selfScale * 50);
		// dropdown->queueConstrainSize();
		ui.setAutocompleteDropdown(std::move(dropdown));
	}

	void TextInput::setMultiline(bool value) {
		multiline = value;
		lastRectangle = {-1, -1, -1, -1};
	}

	bool TextInput::ownsDropdown() const {
		std::shared_ptr<AutocompleteDropdown> dropdown = ui.getAutocompleteDropdown();
		return dropdown && dropdown->checkParent(*this);
	}

	void TextInput::hideDropdown() const {
		ui.setAutocompleteDropdown(nullptr);
	}

	bool TextInput::hasSelection() const {
		return anchor && cursor && *anchor != *cursor;
	}

	void TextInput::copy() {
		if (!hasSelection()) {
			return;
		}

		auto [left, right] = getIterators();
		clip::set_text(std::string(left.base(), right.base()));
	}

	void TextInput::paste() {
		if (std::string pasted; clip::get_text(pasted) && !pasted.empty()) {
			insert(pasted);
		}
	}

	void TextInput::cut() {
		if (hasSelection()) {
			copy();
			eraseCharacter();
		}
	}

	void TextInput::selectAll() {
		ensureCursor();
		ensureAnchor();
		cursor->goStart(false);
		anchor->goEnd(false);
	}

	size_t & TextInput::getLineCount() const {
		if (!cachedLineCount) {
			cachedLineCount = 1 + std::count(text.begin(), text.end(), '\n');
		}
		return *cachedLineCount;
	}

	size_t TextInput::getLastLineNumber() const {
		const size_t count = getLineCount();
		return count == 0? 0 : count - 1;
	}

	size_t TextInput::getColumnCount(size_t line) const {
		if (!cachedColumnCounts) {
			setCachedColumnCounts();
		}

		if (cachedColumnCounts->empty() && line == 0) {
			return 0;
		}

		return cachedColumnCounts->at(line);
	}

	void TextInput::setCachedColumnCounts() const {
		std::vector<size_t> &counts = cachedColumnCounts.emplace();
		counts.reserve(cachedLineCount.value_or(16));
		size_t current_count = 0;
		for (gunichar character: text) {
			if (character == '\n') {
				counts.emplace_back(current_count);
				current_count = 0;
			} else {
				++current_count;
			}
		}
		counts.emplace_back(current_count);
	}

	UStringSpan TextInput::getLineSpan(size_t line_number, size_t max_length) const {
		if (text.empty()) {
			return {text.end(), text.end()};
		}

		if (!cachedColumnCounts) {
			setCachedColumnCounts();
		}

		using iterator = Glib::ustring::const_iterator;

		iterator begin = text.begin();

		for (size_t i = 0; i < line_number; ++i) {
			std::advance(begin, cachedColumnCounts->at(i) + 1); // + 1 to account for the newline
		}

		iterator end = std::next(begin, std::min(max_length, cachedColumnCounts->at(line_number)));

		return {begin, end};
	}

	float TextInput::getCursorHeight() const {
		return getTexter().getIHeight() * TextRenderer::getLineHeight() * getTextScale();
	}

	float TextInput::getTextWidth() const {
		if (!textWidth) {
			TextRenderer &texter = getTexter();
			const float text_scale = getTextScale();

			textWidth = 0;
			widestLine = 0;
			auto end = text.end();
			auto start = text.begin();
			auto last = start;
			size_t line = 0;

			while (start != end) {
				while (last != end && *last != '\n') {
					++last;
				}

				const float width = texter.textWidth(UStringSpan(start, last), text_scale);

				if (*textWidth < width) {
					*textWidth = width;
					widestLine = line;
				}

				if (last == end) {
					break;
				}

				start = ++last;
				++line;
			}
		}

		return *textWidth;
	}

	float TextInput::getTextHeight() const {
		if (!textHeight) {
			textHeight = getTexter().textHeight(text, getTextScale(), -1) + getCursorHeight();
		}

		return *textHeight;
	}

	float TextInput::getLineWidth(size_t line_number) const {
		if (!lineWidths) {
			lineWidths.emplace(getLineCount());
		}

		std::optional<float> &width = lineWidths->at(line_number);
		if (!width) {
			width.emplace(getTexter().textWidth(getLineSpan(line_number), getTextScale()));
		}

		return *width;
	}

	std::pair<UString::iterator, UString::iterator> TextInput::getIterators() const {
		auto [left, right] = getCursors();
		return {left->iterator, right->iterator};
	}

	std::pair<const TextCursor *, const TextCursor *> TextInput::getCursors() const {
		assert(cursor.has_value());

		if (anchor) {
			std::pair out{&*cursor, &*anchor};
			if (anchor->position < cursor->position) {
				std::swap(out.first, out.second);
			}
			return out;
		}

		return {&*cursor, &*cursor};
	}

	TextRenderer & TextInput::getTexter() const {
		return ui.window.textRenderer;
	}

	bool TextInput::ensureCursor() {
		return ensureCursor(cursor);
	}

	bool TextInput::ensureAnchor() {
		return ensureCursor(anchor);
	}

	bool TextInput::ensureCursor(std::optional<TextCursor> &target) {
		if (!target) {
			target.emplace(*this, true, text.begin());
			return true;
		}

		return false;
	}

	void TextInput::clearCachedData() const {
		cachedColumnCounts.reset();
		cachedLineCount.reset();
		widestLine.reset();
		textWidth.reset();
		textHeight.reset();
		lineWidths.reset();
	}

	void TextInput::renderSelection(RectangleRenderer &rectangler) {
		const float cursor_height = getCursorHeight();
		const float text_scale = getTextScale();
		const float start = thickness * getScale();
		const float pixel = start / 2;

		const Color fg = focused? focusedCursorColor : cursorColor;
		const Color bg = fg.withAlpha(0.25);
		const auto [left, right] = getCursors();

		const float left_x = left->getXPosition();
		const float left_y = multiline? left->getYPosition() : start;
		const float right_x = right->getXPosition();
		const float right_y = multiline? right->getYPosition() : start;

		if (left->lineNumber == right->lineNumber) {
			const float width = (right->xOffset - left->xOffset) * text_scale - pixel;
			rectangler(bg, left_x + pixel, left_y, width, cursor_height);
		} else {
			const float full_width = std::max<float>(getTextWidth(), lastRectangle.width);

			{
				const float width = full_width - left->xOffset * text_scale - pixel;
				rectangler(bg, left_x + pixel, left_y, width, cursor_height);
			}

			const float middle_x = getPadding() - xOffset * getScale();
			float middle_y = left_y;
			for (size_t line = left->lineNumber + 1; line < right->lineNumber; ++line) {
				rectangler(bg, middle_x, middle_y += cursor_height, full_width, cursor_height);
			}

			{
				const float width = right->xOffset * text_scale;
				rectangler(bg, middle_x, right_y, width, cursor_height);
			}
		}

		rectangler(fg, left_x, left_y, pixel, cursor_height);
		rectangler(fg, right_x, right_y, pixel, cursor_height);

		if (!left->atLineEnd()) {
			rectangler(fg, left_x + pixel, left_y,                         pixel, pixel);
			rectangler(fg, left_x + pixel, left_y + cursor_height - pixel, pixel, pixel);
		}

		if (!right->atLineBeginning()) {
			rectangler(fg, right_x - pixel, right_y,                         pixel, pixel);
			rectangler(fg, right_x - pixel, right_y + cursor_height - pixel, pixel, pixel);
		}
	}

	std::pair<int, int> TextInput::getLineAndColumn(int x, int y) const {
		if (text.empty()) {
			return {0, 0};
		}

		const float scale = getScale();
		const float start = thickness * scale;
		const float cursor_height = getCursorHeight();

		const float text_y = 2 * start - yOffset * scale;
		int line_number = std::max<int>(0, (y - text_y) / cursor_height);

		if (size_t line_count = getLineCount(); static_cast<size_t>(line_number) >= line_count) {
			line_number = static_cast<int>(getLastLineNumber());
		}

		int column = -1;

		if (size_t column_count = getColumnCount(line_number); column_count > 0) {
			// TODO: monospace optimizations
			const float text_scale = getTextScale();
			float text_left = start - xOffset * scale;

			if (x <= text_left) {
				column = 0;
			} else {
				TextRenderer &texter = getTexter();
				for (int i = 0; gunichar character: getLineSpan(static_cast<size_t>(line_number))) {
					const float width = texter.textWidth(character, text_scale);
					const float text_right = text_left + width;
					if (text_left <= x && x <= text_right) {
						column = i;
						break;
					}
					text_left = text_right;
					++i;
				}

				if (column == -1) {
					column = column_count;
				}
			}
		} else {
			column = 0;
		}

		return std::pair{line_number, column};
	}

	void TextInput::setAnchorAt(int x, int y) {
		const auto [line_number, column] = getLineAndColumn(x, y);
		if (!ensureAnchor()) {
			anchor->reset();
		}
		anchor->goDown(line_number);
		anchor->goRight(column);
		cursor.emplace(*anchor);
	}
}
