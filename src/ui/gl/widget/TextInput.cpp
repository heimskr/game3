#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/gl/Util.h"
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
	TextInput::TextInput(UIContext &ui, float selfScale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness):
		Widget(ui, selfScale),
		HasFixedSize(-1, selfScale * TEXT_INPUT_HEIGHT_FACTOR),
		thickness(thickness),
		borderColor(border_color),
		interiorColor(interior_color),
		textColor(text_color),
		cursorColor(cursor_color),
		focusedCursorColor(cursorColor.darken(3)) {}

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

		if (cursorFixQueued) {
			fixCursorOffset();
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

		rectangler(focused? focusedCursorColor : cursorColor, getCursorPosition(), start, start / 2, interior.height - 2 * start);

		texter(text, TextRenderOptions{
			.x = start - xOffset * getScale(),
			.y = getScale() * 2,
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
			case GLFW_KEY_UP:
				goStart();
				return true;

			case GLFW_KEY_END:
			case GLFW_KEY_DOWN:
				goEnd();
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
				int line_count = 1;
				if (multiline) {
					for (gunichar character: text) {
						if (character == '\n') {
							++line_count;
						}
					}
				}

				minimum = border;
				natural = border + renderers.text.textHeight(text, getTextScale(), for_width - border) * line_count;
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
		if (dropdown && dropdown->checkParent(*this))
			ui.setAutocompleteDropdown(nullptr);
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

		goEnd();
	}

	UString TextInput::clear() {
		UString out = std::move(text);
		lineNumber = 0;
		columnNumber = 0;
		cursorIterator = text.begin();
		xOffset = 0;
		cursorXOffset = 0;
		return out;
	}

	void TextInput::insert(uint32_t character) {
		if (characterFilter && !characterFilter(character, cursorIterator)) {
			return;
		}

		cursorIterator = std::next(text.insert(cursorIterator, character));
		++textPosition;
		if (character == '\n') {
			++lineNumber;
			columnNumber = 0;
			cursorXOffset = 0;
			xOffset = 0;
			++getLineCount();
			cachedColumnCounts.reset();
		} else {
			if (cachedColumnCounts) {
				++cachedColumnCounts->at(lineNumber);
			}
			++columnNumber;
			adjustCursorOffset(ui.getRenderers(0).text.textWidth(character));
		}
	}

	void TextInput::eraseWord() {
		if (atBeginning()) {
			return;
		}

		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.

		if (isWhitespace(cursorIterator)) {
			do {
				eraseCharacter();
			} while (cursorIterator != text.begin() && isWhitespace(cursorIterator));

			while (cursorIterator != text.begin() && !isStopChar(cursorIterator)) {
				eraseCharacter();
			}

			return;
		}

		if (isStopChar(cursorIterator)) {
			do {
				eraseCharacter();
			} while (cursorIterator != text.begin() && isStopChar(cursorIterator));

			return;
		}

		do {
			eraseCharacter();
		} while (cursorIterator != text.begin() && !isStopChar(cursorIterator));

		while (cursorIterator != text.begin() && isWhitespace(cursorIterator)) {
			eraseCharacter();
		}
	}

	void TextInput::eraseCharacter() {
		if (atBeginning()) {
			return;
		}

		if (*--cursorIterator == '\n') {
			columnNumber = getColumnCount(--lineNumber);
			--getLineCount();
			cachedColumnCounts.reset();
			cursorXOffset = ui.getRenderers(0).text.textWidth(getLineSpan(lineNumber));
			fixCursorOffset();
		} else {
			if (cachedColumnCounts) {
				--cachedColumnCounts->at(lineNumber);
			}
			--columnNumber;
			adjustCursorOffset(-ui.getRenderers(0).text.textWidth(*cursorIterator));
		}

		--textPosition;
		cursorIterator = text.erase(cursorIterator);
	}

	void TextInput::eraseForward() {
		if (!text.empty() && cursorIterator != text.end()) {
			cursorIterator = text.erase(cursorIterator);
		}
	}

	void TextInput::goLeft(size_t count) {
		RendererContext renderers = ui.getRenderers(0);

		for (size_t i = 0; i < count && cursorIterator != text.begin(); ++i) {
			if (*--cursorIterator == '\n') {
				columnNumber = getColumnCount(--lineNumber);
				cursorXOffset = renderers.text.textWidth(getLineSpan(lineNumber));
				fixCursorOffset();
			} else {
				--columnNumber;
				adjustCursorOffset(-renderers.text.textWidth(UStringSpan(cursorIterator, std::next(cursorIterator))));
			}
			--textPosition;
		}
	}

	void TextInput::goRight(size_t count) {
		RendererContext renderers = ui.getRenderers(0);

		for (size_t i = 0; i < count && cursorIterator != text.end(); ++i) {
			auto old_iterator = cursorIterator++;
			if (*old_iterator == '\n') {
				++lineNumber;
				columnNumber = 0;
				cursorXOffset = 0;
			} else {
				++columnNumber;
				adjustCursorOffset(renderers.text.textWidth(UStringSpan(old_iterator, cursorIterator)));
			}
			++textPosition;
		}
	}

	void TextInput::goStart() {
		lineNumber = 0;
		columnNumber = 0;
		cursorIterator = text.begin();
		xOffset = 0;
		setCursorOffset(0);
	}

	void TextInput::goEnd() {
		lineNumber = getLastLineNumber();
		columnNumber = getColumnCount(lineNumber);
		cursorIterator = text.end();
		textPosition = text.size();
		xOffset = 0;
		setCursorOffset(ui.getRenderers(0).text.textWidth(getLineSpan(lineNumber)));
	}

	void TextInput::autocomplete(const UString &completion) {
		setText(completion);
		onAcceptSuggestion(*this, completion);
	}

	float TextInput::getTextScale() const {
		return getScale() / 16;
	}

	float TextInput::getXPadding() const {
		return thickness * getScale();
	}

	float TextInput::getBoundary() const {
		return lastRectangle.width - getXPadding();
	}

	float TextInput::getCursorPosition() const {
		return getXPadding() - xOffset * getScale() + cursorXOffset * getTextScale();
	}

	void TextInput::adjustCursorOffset(float offset) {
		cursorXOffset += offset;
		fixCursorOffset();
	}

	void TextInput::setCursorOffset(float new_offset) {
		cursorXOffset = new_offset;
		fixCursorOffset();
	}

	void TextInput::fixCursorOffset() {
		if (lastRectangle.width < 0) {
			cursorFixQueued = true;
			return;
		}

		const float visual = getCursorPosition();
		const float boundary = getBoundary();

		if (visual > boundary) {
			xOffset += (visual - boundary + getXPadding() * 2) / getScale();
		} else if (visual < getXPadding()) {
			xOffset -= (getXPadding() - visual) / getScale();
		}

		cursorFixQueued = false;
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
		if (relevant.empty())
			return;

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
		return lineNumber == 0 && columnNumber == 0;
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

	UStringSpan TextInput::getLineSpan(size_t line_number) const {
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

		iterator end = std::next(begin, cachedColumnCounts->at(line_number));

		return {begin, end};
	}
}
