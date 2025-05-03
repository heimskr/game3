#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/gl/Util.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/widget/Tooltip.h"
#include "util/Util.h"

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
	TextCursor & TextCursor::operator--() {
		if (position == 0) {
			return *this;
		}



		return *this;
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
		cursor(text.begin()) {}

	TextInput::TextInput(UIContext &ui, float selfScale, Color border_color, Color interior_color, Color text_color, Color cursor_color):
		TextInput(ui, selfScale, border_color, interior_color, text_color, cursor_color, DEFAULT_THICKNESS) {}

	TextInput::TextInput(UIContext &ui, float selfScale, float thickness):
		TextInput(ui, selfScale, DEFAULT_BORDER_COLOR, DEFAULT_TEXTINPUT_INTERIOR_COLOR, DEFAULT_TEXT_COLOR, DEFAULT_CURSOR_COLOR, thickness) {}

	TextInput::TextInput(UIContext &ui, float selfScale):
		TextInput(ui, selfScale, DEFAULT_THICKNESS) {}

	void TextInput::render(const RendererContext &renderers, float x, float y, float width, float height) {
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

		if (offsetFixQueued) {
			fixXOffset();
			fixYOffset();
		}

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const float start = thickness * getScale();
		// TODO: check for negative sizes
		auto interior = Rectangle(x + start, y + start, width - 2 * start, height - 2 * start);

		rectangler(borderColor, Rectangle(x, y, width, height * 0.6));
		rectangler(borderColor.darken(), Rectangle(x, y + height * 0.6, width, height * 0.4));
		rectangler(interiorColor, interior);

		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		if (multiline) {
			float cursor_height = getCursorHeight(texter);
			rectangler(focused? focusedCursorColor : cursorColor, getCursorXPosition(), getCursorYPosition(), start / 2, cursor_height);
		} else {
			rectangler(focused? focusedCursorColor : cursorColor, getCursorXPosition(), start, start / 2, interior.height - 2 * start);
		}

		texter(text, TextRenderOptions{
			.x = start - xOffset * getScale(),
			.y = 2 * start - yOffset * getScale(),
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
			return true;
		}

		return false;
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

		if (modifiers.ctrl) {
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
				goLeft();
				return true;

			case GLFW_KEY_RIGHT:
				goRight();
				return true;

			case GLFW_KEY_HOME:
				goStart(!modifiers.ctrl);
				return true;

			case GLFW_KEY_END:
				goEnd(!modifiers.ctrl);
				return true;

			case GLFW_KEY_UP:
				goUp();
				return true;

			case GLFW_KEY_DOWN:
				goDown();
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

	bool TextInput::charPressed(uint32_t codepoint, Modifiers) {
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
						yOffset = std::min<float>(height, yOffset - scroll_speed * y_delta);
					}
					return true;
				}

				const float new_offset = std::max<float>(0, yOffset - scroll_speed * y_delta);
				if (yOffset != new_offset || yOffset == 0) {
					yOffset = new_offset;
					return true;
				}
			}
		}

		if (x_delta != 0) {
			const float width = (getTextWidth() - lastRectangle.width) / getScale() + 4 * thickness;
			if (width >= 0 || xOffset != 0) {
				if (x_delta < 0) {
					if (width >= 0) {
						xOffset = std::min<float>(width, xOffset - scroll_speed * x_delta);
					}
					return true;
				}

				const float new_offset = std::max<float>(0, xOffset - scroll_speed * x_delta);
				if (xOffset != new_offset || xOffset == 0) {
					xOffset = new_offset;
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
				natural = border + renderers.text.textHeight(text, getTextScale(), multiline? -1 : for_width - border) + getCursorHeight(renderers.text);
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

		goEnd(false);
	}

	UString TextInput::clear() {
		UString out = std::move(text);
		cursor = text.begin();
		anchor.reset();
		xOffset = 0;
		yOffset = 0;
		return out;
	}

	void TextInput::insert(uint32_t character) {
		if (characterFilter && !characterFilter(character, cursor.iterator)) {
			return;
		}

		cursor.iterator = std::next(text.insert(cursor.iterator, character));
		++cursor.position;
		if (character == '\n') {
			++cursor.lineNumber;
			cursor.columnNumber = 0;
			cursor.xOffset = 0;
			xOffset = 0;
			++getLineCount();
			cachedColumnCounts.reset();
			fixYOffset();
			textHeight.reset();
			textWidth.reset();
			widestLine.reset();
		} else {
			if (cachedColumnCounts) {
				++cachedColumnCounts->at(cursor.lineNumber);
			}
			++cursor.columnNumber;
			const float width = ui.getRenderers(0).text.textWidth(character);
			cursor.xOffset += width;
			if (widestLine == cursor.lineNumber) {
				textWidth.reset();
			}
			fixXOffset();
		}
	}

	void TextInput::eraseWord() {
		if (atBeginning()) {
			return;
		}

		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.

		if (isWhitespace(cursor.iterator)) {
			do {
				eraseCharacter();
			} while (cursor.iterator != text.begin() && isWhitespace(cursor.iterator));

			while (cursor.iterator != text.begin() && !isStopChar(cursor.iterator)) {
				eraseCharacter();
			}

			return;
		}

		if (isStopChar(cursor.iterator)) {
			do {
				eraseCharacter();
			} while (cursor.iterator != text.begin() && isStopChar(cursor.iterator));

			return;
		}

		do {
			eraseCharacter();
		} while (cursor.iterator != text.begin() && !isStopChar(cursor.iterator));

		while (cursor.iterator != text.begin() && isWhitespace(cursor.iterator)) {
			eraseCharacter();
		}
	}

	void TextInput::eraseCharacter() {
		if (atBeginning()) {
			return;
		}

		TextRenderer &texter = ui.getRenderers(0).text;

		if (*--cursor.iterator == '\n') {
			cursor.columnNumber = getColumnCount(--cursor.lineNumber);
			--getLineCount();
			cachedColumnCounts.reset();
			fixYOffset();
			cursor.xOffset = texter.textWidth(getLineSpan(cursor.lineNumber));
			textWidth.reset();
			textHeight.reset();
			widestLine.reset();
			fixXOffset();
		} else {
			if (cachedColumnCounts) {
				--cachedColumnCounts->at(cursor.lineNumber);
			}
			--cursor.columnNumber;
			cursor.xOffset -= texter.textWidth(*cursor.iterator);
			if (widestLine == cursor.lineNumber) {
				textWidth.reset();
				widestLine.reset();
			}
			fixXOffset();
		}

		--cursor.position;
		cursor.iterator = text.erase(cursor.iterator);
	}

	void TextInput::eraseForward() {
		if (!text.empty() && cursor.iterator != text.end()) {
			if (*cursor.iterator == '\n') {
				--getLineCount();
				cachedColumnCounts.reset();
				textHeight.reset();
				textWidth.reset();
				widestLine.reset();
			} else {
				if (cachedColumnCounts) {
					--cachedColumnCounts->at(cursor.lineNumber);
				}

				if (widestLine == cursor.lineNumber) {
					textWidth.reset();
					widestLine.reset();
				}
			}
			cursor.iterator = text.erase(cursor.iterator);
		}
	}

	void TextInput::goLeft(size_t count) {
		RendererContext renderers = ui.getRenderers(0);

		for (size_t i = 0; i < count && cursor.iterator != text.begin(); ++i) {
			if (*--cursor.iterator == '\n') {
				cursor.columnNumber = getColumnCount(--cursor.lineNumber);
				cursor.xOffset = renderers.text.textWidth(getLineSpan(cursor.lineNumber));
				fixYOffset();
			} else {
				--cursor.columnNumber;
				cursor.xOffset -= renderers.text.textWidth(UStringSpan(cursor.iterator, std::next(cursor.iterator)));
			}
			--cursor.position;
			fixXOffset();
		}
	}

	void TextInput::goRight(size_t count) {
		TextRenderer &texter = ui.getRenderers(0).text;

		for (size_t i = 0; i < count && cursor.iterator != text.end(); ++i) {
			auto old_iterator = cursor.iterator++;
			if (*old_iterator == '\n') {
				++cursor.lineNumber;
				cursor.columnNumber = 0;
				cursor.xOffset = 0;
				xOffset = 0;
			} else {
				++cursor.columnNumber;
				cursor.xOffset += texter.textWidth(UStringSpan(old_iterator, cursor.iterator));
				fixXOffset();
			}
			++cursor.position;
		}
	}

	void TextInput::goStart(bool within_line) {
		if (within_line) {
			cursor.position -= cursor.columnNumber;
			while (cursor.columnNumber > 0) {
				--cursor.columnNumber;
				--cursor.iterator;
			}
		} else {
			cursor.lineNumber = 0;
			cursor.columnNumber = 0;
			cursor.iterator = text.begin();
			cursor.position = 0;
		}

		xOffset = 0;
		cursor.xOffset = 0;
	}

	void TextInput::goEnd(bool within_line) {
		if (within_line) {
			const size_t column_count = getColumnCount(cursor.lineNumber);
			cursor.position += column_count - cursor.columnNumber;
			while (cursor.columnNumber < column_count) {
				++cursor.columnNumber;
				++cursor.iterator;
			}
		} else {
			cursor.lineNumber = getLastLineNumber();
			cursor.columnNumber = getColumnCount(cursor.lineNumber);
			cursor.iterator = text.end();
			cursor.position = text.size();
		}

		cursor.xOffset = ui.getRenderers(0).text.textWidth(getLineSpan(cursor.lineNumber));
		fixXOffset();
	}

	void TextInput::goUp() {
		if (cursor.lineNumber == 0) {
			if (!atBeginning()) {
				goStart(false);
			}
		} else {
			while (0 < cursor.position) {
				--cursor.position;
				if (*--cursor.iterator == '\n') {
					break;
				}
			}

			const size_t old_column_number = std::exchange(cursor.columnNumber, getColumnCount(--cursor.lineNumber));

			while (cursor.columnNumber > old_column_number) {
				--cursor.columnNumber;
				--cursor.iterator;
				--cursor.position;
			}

			auto last = cursor.iterator;
			auto start = std::prev(last, cursor.columnNumber);

			cursor.xOffset = ui.getRenderers(0).text.textWidth(UStringSpan(start, last));
			fixXOffset();
			fixYOffset();
		}
	}

	void TextInput::goDown() {
		if (cursor.lineNumber + 1 >= getLineCount()) {
			if (!atEnd()) {
				goEnd(false);
			}
		} else {
			while (cursor.position < text.length()) {
				++cursor.position;
				if (*cursor.iterator++ == '\n') {
					break;
				}
			}

			auto start = cursor.iterator;
			auto last = start;

			size_t i = 0;

			for (; i < cursor.columnNumber && cursor.position < text.length(); ++i) {
				++cursor.position;
				if (*cursor.iterator++ == '\n') {
					break;
				}
				++last;
			}

			cursor.columnNumber = i;
			++cursor.lineNumber;
			TextRenderer &texter = ui.getRenderers(0).text;
			cursor.xOffset = texter.textWidth(UStringSpan(start, last));
			fixXOffset();
			fixYOffset();
		}
	}

	void TextInput::autocomplete(const UString &completion) {
		setText(completion);
		onAcceptSuggestion(*this, completion);
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

	float TextInput::getCursorXPosition() const {
		return getPadding() - xOffset * getScale() + cursor.xOffset * getTextScale();
	}

	float TextInput::getCursorYPosition() const {
		return getPadding() - yOffset * getScale() + cursor.lineNumber * getCursorHeight(ui.getRenderers(0).text);
	}

	void TextInput::fixXOffset() {
		if (lastRectangle.width < 0) {
			offsetFixQueued = true;
			return;
		}

		const float visual = getCursorXPosition();
		const float boundary = getXBoundary();
		const float padding = getPadding();

		if (visual > boundary) {
			xOffset += (visual - boundary + padding * 2) / getScale();
		} else if (visual < padding) {
			xOffset -= (padding - visual) / getScale();
		}

		offsetFixQueued = false;
	}

	void TextInput::fixYOffset() {
		if (lastRectangle.height < 0) {
			offsetFixQueued = true;
			return;
		}

		const float visual = getCursorYPosition();
		const float boundary = getYBoundary();
		const float padding = getPadding();
		const float cursor_height = getCursorHeight(ui.getRenderers(0).text);

		if (visual + cursor_height > boundary) {
			yOffset += (visual - boundary + padding * 2 + cursor_height) / getScale();
		} else if (visual < padding) {
			yOffset -= (padding - visual) / getScale();
		}

		offsetFixQueued = false;
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

	bool TextInput::atBeginning() const {
		return cursor.lineNumber == 0 && cursor.columnNumber == 0;
	}

	bool TextInput::atEnd() const {
		return cursor.iterator == text.end();
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

	float TextInput::getCursorHeight(const TextRenderer &texter) const {
		return texter.getIHeight() * TextRenderer::getLineHeight() * getTextScale();
	}

	float TextInput::getTextWidth() const {
		if (!textWidth) {
			TextRenderer &texter = ui.getRenderers(0).text;
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
			TextRenderer &texter = ui.getRenderers(0).text;
			textHeight = texter.textHeight(text, getTextScale(), -1) + getCursorHeight(texter);
		}

		return *textHeight;
	}
}
