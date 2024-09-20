#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr Game3::Color DEFAULT_BORDER_COLOR{"#926641"};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{"#341903"};
	constexpr Game3::Color DEFAULT_CURSOR_COLOR{"#927a66"};
	constexpr float DEFAULT_THICKNESS = 1;

	bool isStopChar(Game3::UString::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max())
			return false;

		static const std::string_view stops = "_.,/-=+ \t\n";
		return stops.find(static_cast<char>(unicharacter)) != std::string_view::npos;
	}

	bool isWhitespace(Game3::UString::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max())
			return false;

		static const std::string_view stops = " \t\n";
		return stops.find(static_cast<char>(unicharacter)) != std::string_view::npos;
	}
}

namespace Game3 {
	TextInput::TextInput(UIContext &ui, float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness):
		Widget(ui, scale),
		HasFixedSize(-1, scale * TEXT_INPUT_HEIGHT_FACTOR),
		thickness(thickness),
		borderColor(border_color),
		interiorColor(interior_color),
		textColor(text_color),
		cursorColor(cursor_color),
		focusedCursorColor(cursorColor.darken(3)) {}

	TextInput::TextInput(UIContext &ui, float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color):
		TextInput(ui, scale, border_color, interior_color, text_color, cursor_color, DEFAULT_THICKNESS) {}

	TextInput::TextInput(UIContext &ui, float scale, float thickness):
		TextInput(ui, scale, DEFAULT_BORDER_COLOR, DEFAULT_TEXTINPUT_INTERIOR_COLOR, DEFAULT_TEXT_COLOR, DEFAULT_CURSOR_COLOR, thickness) {}

	TextInput::TextInput(UIContext &ui, float scale):
		TextInput(ui, scale, DEFAULT_THICKNESS) {}

	void TextInput::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (0 < fixedHeight)
			height = fixedHeight;

		Widget::render(renderers, x, y, width, height);

		if (cursorFixQueued)
			fixCursorOffset();

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const float start = thickness * scale;
		// TODO: check for negative sizes
		const Rectangle interior(x + start, y + start, width - 2 * start, height - 2 * start);

		rectangler(borderColor, x, y, width, height * 0.6);
		rectangler(borderColor.darken(), x, y + height * 0.6, width, height * 0.4);
		rectangler(interiorColor, interior);

		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		rectangler(focused? focusedCursorColor : cursorColor, getCursorPosition(), start, start / 2, interior.height - 2 * start);

		texter(text, TextRenderOptions{
			.x = start - xOffset * scale,
			.y = scale * 2,
			.scaleX = getTextScale(),
			.scaleY = getTextScale(),
			.color = textColor,
			.alignTop = true,
			.shadow{0, 0, 0, 0},
			.ignoreNewline = true,
		});
	}

	bool TextInput::click(int button, int, int) {
		if (button == 1) {
			ui.focusWidget(shared_from_this());
			return true;
		}

		return false;
	}

	bool TextInput::keyPressed(uint32_t character, Modifiers modifiers) {
		if (modifiers.ctrl) {
			switch (character) {
				case GDK_KEY_BackSpace:
					eraseWord();
					changed();
					break;

				default:
					break;
			}

			// Ignore other ctrl sequences.
			return true;
		}

		switch (character) {
			case GDK_KEY_BackSpace:
				eraseCharacter();
				changed();
				return true;

			case GDK_KEY_Delete:
				eraseForward();
				changed();
				return true;

			case GDK_KEY_Left:
				goLeft();
				return true;

			case GDK_KEY_Right:
				goRight();
				return true;

			case GDK_KEY_Home:
			case GDK_KEY_Up:
				goStart();
				return true;

			case GDK_KEY_End:
			case GDK_KEY_Down:
				goEnd();
				return true;

			case GDK_KEY_Shift_L:
			case GDK_KEY_Shift_R:
			case GDK_KEY_Control_L:
			case GDK_KEY_Control_R:
			case GDK_KEY_Alt_L:
			case GDK_KEY_Alt_R:
			case GDK_KEY_Super_L:
			case GDK_KEY_Super_R:
			case GDK_KEY_Menu:
				return true;

			case GDK_KEY_Escape:
				if (ownsDropdown())
					hideDropdown();
				else
					ui.unfocus();
				return true;

			case GDK_KEY_Return:
			case GDK_KEY_KP_Enter:
				onSubmit(*this, text);
				return true;

			default:
				break;
		}

		insert(static_cast<gunichar>(character));
		changed();
		return true;
	}

	SizeRequestMode TextInput::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void TextInput::measure(const RendererContext &renderers, Orientation orientation, float for_width, float, float &minimum, float &natural) {
		const float border = 2 * thickness * scale;

		if (orientation == Orientation::Horizontal) {
			if (0 < fixedWidth) {
				minimum = natural = fixedWidth;
			} else {
				minimum = border;
				natural = border + renderers.text.textWidth(text, getTextScale());
			}
		} else {
			if (0 < fixedHeight) {
				minimum = natural = fixedHeight;
			} else {
				minimum = border;
				natural = border + renderers.text.textHeight(text, getTextScale(), for_width - border);
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
				if (suggestion.length() > text.length() && suggestion.raw().starts_with(raw))
					relevant.push_back(suggestion);
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
		cursor = 0;
		cursorIterator = text.begin();
		xOffset = 0;
		cursorXOffset = 0;
		return out;
	}

	void TextInput::insert(gunichar character) {
		cursorIterator = text.insert(cursorIterator, character);
		++cursorIterator;
		++cursor;
		adjustCursorOffset(ui.getRenderers().text.textWidth(UString(1, character)));
	}

	void TextInput::eraseWord() {
		if (cursor == 0)
			return;

		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.

		if (isWhitespace(cursorIterator)) {
			do {
				eraseCharacter();
			} while (cursorIterator != text.begin() && isWhitespace(cursorIterator));

			while (cursorIterator != text.begin() && !isStopChar(cursorIterator))
				eraseCharacter();

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

		while (cursorIterator != text.begin() && isWhitespace(cursorIterator))
			eraseCharacter();
	}

	void TextInput::eraseCharacter() {
		if (cursor == 0)
			return;

		adjustCursorOffset(-ui.getRenderers().text.textWidth(text.substr(--cursor, 1)));
		cursorIterator = text.erase(--cursorIterator);
	}

	void TextInput::eraseForward() {
		if (!text.empty() && cursorIterator != text.end())
			cursorIterator = text.erase(cursorIterator);
	}

	void TextInput::goLeft(size_t count) {
		RendererContext renderers = ui.getRenderers();
		UString piece;

		for (size_t i = 0; i < count && cursorIterator != text.begin(); ++i) {
			piece = text.substr(--cursor, 1);
			adjustCursorOffset(-renderers.text.textWidth(piece));
			--cursorIterator;
		}
	}

	void TextInput::goRight(size_t count) {
		RendererContext renderers = ui.getRenderers();
		UString piece;

		for (size_t i = 0; i < count && cursorIterator != text.end(); ++i) {
			piece = text.substr(cursor++, 1);
			adjustCursorOffset(renderers.text.textWidth(piece));
			++cursorIterator;
		}
	}

	void TextInput::goStart() {
		cursor = 0;
		cursorIterator = text.begin();
		xOffset = 0;
		setCursorOffset(0);
	}

	void TextInput::goEnd() {
		cursor = text.length();
		cursorIterator = text.end();
		xOffset = 0;
		setCursorOffset(ui.getRenderers().text.textWidth(text));
	}

	void TextInput::autocomplete(const UString &completion) {
		setText(completion);
		onAcceptSuggestion(*this, completion);
	}

	float TextInput::getTextScale() const {
		return scale / 16;
	}

	float TextInput::getXPadding() const {
		return thickness * scale;
	}

	float TextInput::getBoundary() const {
		return lastRectangle.width - getXPadding();
	}

	float TextInput::getCursorPosition() const {
		return getXPadding() - xOffset * scale + cursorXOffset * getTextScale();
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

		if (visual > boundary)
			xOffset += (visual - boundary + getXPadding() * 2) / scale;
		else if (visual < getXPadding())
			xOffset -= (getXPadding() - visual) / scale;

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

		auto dropdown = std::make_shared<AutocompleteDropdown>(ui, scale);
		dropdown->init();
		dropdown->setParent(std::dynamic_pointer_cast<Autocompleter>(shared_from_this()));
		dropdown->setOrigin({lastRectangle.x, lastRectangle.y + lastRectangle.height});
		dropdown->setSuggestions(std::move(relevant));
		dropdown->setFixedSize(lastRectangle.width, scale * 50);
		// dropdown->queueConstrainSize();
		ui.setAutocompleteDropdown(std::move(dropdown));
	}

	bool TextInput::ownsDropdown() const {
		std::shared_ptr<AutocompleteDropdown> dropdown = ui.getAutocompleteDropdown();
		return dropdown && dropdown->checkParent(*this);
	}

	void TextInput::hideDropdown() const {
		ui.setAutocompleteDropdown(nullptr);
	}
}
