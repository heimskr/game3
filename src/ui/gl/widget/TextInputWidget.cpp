#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/widget/TooltipWidget.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace {
	constexpr Game3::Color DEFAULT_BORDER_COLOR{"#926641"};
	constexpr Game3::Color DEFAULT_INTERIOR_COLOR{"#fdeed3"};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{"#341903"};
	constexpr Game3::Color DEFAULT_CURSOR_COLOR{"#927a66"};
	constexpr float DEFAULT_THICKNESS = 1;

	bool isStopChar(Glib::ustring::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max())
			return false;

		static const std::string_view stops = "_.,/-=+ \t\n";
		return stops.find(static_cast<char>(unicharacter)) != std::string_view::npos;
	}

	bool isWhitespace(Glib::ustring::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max())
			return false;

		static const std::string_view stops = " \t\n";
		return stops.find(static_cast<char>(unicharacter)) != std::string_view::npos;
	}
}

namespace Game3 {
	TextInputWidget::TextInputWidget(float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color, float thickness):
		Widget(scale), thickness(thickness), borderColor(border_color), interiorColor(interior_color), textColor(text_color), cursorColor(cursor_color) {}

	TextInputWidget::TextInputWidget(float scale, Color border_color, Color interior_color, Color text_color, Color cursor_color):
		TextInputWidget(scale, border_color, interior_color, text_color, cursor_color, DEFAULT_THICKNESS) {}

	TextInputWidget::TextInputWidget(float scale, float thickness):
		TextInputWidget(scale, DEFAULT_BORDER_COLOR, DEFAULT_INTERIOR_COLOR, DEFAULT_TEXT_COLOR, DEFAULT_CURSOR_COLOR, thickness) {}

	TextInputWidget::TextInputWidget(float scale):
		TextInputWidget(scale, DEFAULT_THICKNESS) {}

	void TextInputWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (0 < fixedHeight)
			height = fixedHeight;

		Widget::render(ui, renderers, x, y, width, height);

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

		rectangler(cursorColor, getCursorPosition(), start, start / 2, interior.height - 2 * start);

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

	bool TextInputWidget::click(UIContext &ui, int button, int, int) {
		if (button == 1) {
			ui.focusWidget(weak_from_this());
			return true;
		}

		return false;
	}

	bool TextInputWidget::keyPressed(UIContext &ui, uint32_t character, Modifiers modifiers) {
		if (modifiers.ctrl) {
			switch (character) {
				case GDK_KEY_BackSpace:
					eraseWord(ui);
					break;

				default:
					break;
			}

			// Ignore other ctrl sequences.
			return true;
		}

		switch (character) {
			case GDK_KEY_BackSpace:
				eraseCharacter(ui);
				return true;

			case GDK_KEY_Delete:
				eraseForward(ui);
				return true;

			case GDK_KEY_Left:
				goLeft(ui);
				return true;

			case GDK_KEY_Right:
				goRight(ui);
				return true;

			case GDK_KEY_Home:
			case GDK_KEY_Up:
				goStart(ui);
				return true;

			case GDK_KEY_End:
			case GDK_KEY_Down:
				goEnd(ui);
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
				ui.unfocus();
				return true;

			case GDK_KEY_Return:
			case GDK_KEY_KP_Enter:
				if (onSubmit)
					onSubmit(*this, ui);
				return true;

			default:
				break;
		}

		insert(ui, static_cast<gunichar>(character));
		return true;
	}

	float TextInputWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return 0 < fixedHeight? fixedHeight : available_height;
	}

	const Glib::ustring & TextInputWidget::getText() const {
		return text;
	}

	void TextInputWidget::setText(UIContext &ui, Glib::ustring new_text) {
		text = std::move(new_text);
		goEnd(ui);
	}

	Glib::ustring TextInputWidget::clear() {
		Glib::ustring out = std::move(text);
		cursor = 0;
		cursorIterator = text.begin();
		xOffset = 0;
		cursorXOffset = 0;
		return out;
	}

	void TextInputWidget::insert(UIContext &ui, gunichar character) {
		cursorIterator = text.insert(cursorIterator, static_cast<gunichar>(character));
		++cursorIterator;
		++cursor;
		adjustCursorOffset(ui.getRenderers().text.textWidth(Glib::ustring(1, character)));
	}

	void TextInputWidget::eraseWord(UIContext &ui) {
		if (cursor == 0)
			return;

		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.

		if (isWhitespace(cursorIterator)) {
			do {
				eraseCharacter(ui);
			} while (cursorIterator != text.begin() && isWhitespace(cursorIterator));

			while (cursorIterator != text.begin() && !isStopChar(cursorIterator))
				eraseCharacter(ui);

			return;
		}

		if (isStopChar(cursorIterator)) {
			do {
				eraseCharacter(ui);
			} while (cursorIterator != text.begin() && isStopChar(cursorIterator));

			return;
		}

		do {
			eraseCharacter(ui);
		} while (cursorIterator != text.begin() && !isStopChar(cursorIterator));

		while (cursorIterator != text.begin() && isWhitespace(cursorIterator))
			eraseCharacter(ui);
	}

	void TextInputWidget::eraseCharacter(UIContext &ui) {
		if (cursor == 0)
			return;

		adjustCursorOffset(-ui.getRenderers().text.textWidth(text.substr(--cursor, 1)));
		cursorIterator = text.erase(--cursorIterator);
	}

	void TextInputWidget::eraseForward(UIContext &) {
		if (!text.empty() && cursorIterator != text.end())
			cursorIterator = text.erase(cursorIterator);
	}

	void TextInputWidget::goLeft(UIContext &ui, size_t count) {
		RendererContext renderers = ui.getRenderers();
		Glib::ustring piece;

		for (size_t i = 0; i < count && cursorIterator != text.begin(); ++i) {
			piece = text.substr(--cursor, 1);
			adjustCursorOffset(-renderers.text.textWidth(piece));
			--cursorIterator;
		}
	}

	void TextInputWidget::goRight(UIContext &ui, size_t count) {
		RendererContext renderers = ui.getRenderers();
		Glib::ustring piece;

		for (size_t i = 0; i < count && cursorIterator != text.end(); ++i) {
			piece = text.substr(cursor++, 1);
			adjustCursorOffset(renderers.text.textWidth(piece));
			++cursorIterator;
		}
	}

	void TextInputWidget::goStart(UIContext &) {
		cursor = 0;
		cursorIterator = text.begin();
		xOffset = 0;
		setCursorOffset(0);
	}

	void TextInputWidget::goEnd(UIContext &ui) {
		cursor = text.length();
		cursorIterator = text.end();
		xOffset = 0;
		setCursorOffset(ui.getRenderers().text.textWidth(text));
	}

	float TextInputWidget::getTextScale() const {
		return scale / 16;
	}

	float TextInputWidget::getXPadding() const {
		return thickness * scale;
	}

	float TextInputWidget::getBoundary() const {
		return lastRectangle.width - getXPadding();
	}

	float TextInputWidget::getCursorPosition() const {
		return getXPadding() - xOffset * scale + cursorXOffset * getTextScale();
	}

	void TextInputWidget::adjustCursorOffset(float offset) {
		cursorXOffset += offset;
		fixCursorOffset();
	}

	void TextInputWidget::setCursorOffset(float new_offset) {
		cursorXOffset = new_offset;
		fixCursorOffset();
	}

	void TextInputWidget::fixCursorOffset() {
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
}
