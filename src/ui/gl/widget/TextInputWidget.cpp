#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace {
	constexpr Game3::Color DEFAULT_EXTERIOR_COLOR{0, 0, 0, 1};
	constexpr Game3::Color DEFAULT_INTERIOR_COLOR{1, 1, 1, 1};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{0, 0, 0, 1};
	constexpr Game3::Color DEFAULT_CURSOR_COLOR{0, 0, 0, 0.5};
	constexpr float DEFAULT_THICKNESS = 1;

	bool isStopChar(Glib::ustring::iterator cursor) {
		const gunichar unicharacter = *--cursor;

		if (unicharacter > std::numeric_limits<char>::max())
			return false;

		static const std::string_view stops = "_. \t\n";
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
	TextInputWidget::TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, Color cursor_color, float thickness):
		scale(scale), thickness(thickness), exteriorColor(exterior_color), interiorColor(interior_color), textColor(text_color), cursorColor(cursor_color) {
			text = "This is a test";
			cursorIterator = text.end();
			cursor = text.length();
			xOffset = 0;
		}

	TextInputWidget::TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, Color cursor_color):
		TextInputWidget(scale, exterior_color, interior_color, text_color, cursor_color, DEFAULT_THICKNESS) {}

	TextInputWidget::TextInputWidget(float scale, float thickness):
		TextInputWidget(scale, DEFAULT_EXTERIOR_COLOR, DEFAULT_INTERIOR_COLOR, DEFAULT_TEXT_COLOR, DEFAULT_CURSOR_COLOR, thickness) {}

	TextInputWidget::TextInputWidget(float scale):
		TextInputWidget(scale, DEFAULT_THICKNESS) {}

	void TextInputWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const float start = thickness * scale;
		// TODO: check for negative sizes
		const Rectangle interior(x + start, y + start, width - 2 * start, height - 2 * start);

		rectangler(exteriorColor, x, y, width, height);
		rectangler(interiorColor, interior);

		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		rectangler(cursorColor, x - xOffset * scale + scale + start / 2 + cursorXOffset * getTextScale(), y + start, start / 2, interior.height - 1.5 * start);

		texter(text, TextRenderOptions{
			.x = x - xOffset * scale + start,
			.y = y,
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
		switch (character) {
			case GDK_KEY_Return:
				character = '\n';
				break;

			case GDK_KEY_BackSpace:
				if (modifiers.onlyCtrl())
					eraseWord(ui);
				else
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
				goStart(ui);
				return true;

			case GDK_KEY_End:
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

			default:
				break;
		}

		insert(ui, static_cast<gunichar>(character));
		return true;
	}

	float TextInputWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return available_height;
	}

	void TextInputWidget::insert(UIContext &ui, gunichar character) {
		cursorIterator = text.insert(cursorIterator, static_cast<gunichar>(character));
		++cursorIterator;
		++cursor;
		cursorXOffset += ui.getRenderers().text.textWidth(Glib::ustring(1, character));
	}

	void TextInputWidget::eraseWord(UIContext &ui) {
		if (cursor == 0)
			return;

		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.
		if (isStopChar(cursorIterator)) {
			do {
				eraseCharacter(ui);
			} while (cursorIterator != text.begin() && isStopChar(cursorIterator));
		} else {
			do {
				eraseCharacter(ui);
			} while (cursorIterator != text.begin() && !isStopChar(cursorIterator));
			while (cursorIterator != text.begin() && isWhitespace(cursorIterator)) {
				eraseCharacter(ui);
			}
		}
	}

	void TextInputWidget::eraseCharacter(UIContext &ui) {
		if (cursor == 0)
			return;

		cursorXOffset -= ui.getRenderers().text.textWidth(text.substr(--cursor, 1));
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
			cursorXOffset -= renderers.text.textWidth(piece);
			--cursorIterator;
		}
	}

	void TextInputWidget::goRight(UIContext &ui, size_t count) {
		RendererContext renderers = ui.getRenderers();
		Glib::ustring piece;

		for (size_t i = 0; i < count && cursorIterator != text.end(); ++i) {
			piece = text.substr(cursor++, 1);
			cursorXOffset += renderers.text.textWidth(piece);
			++cursorIterator;
		}
	}

	void TextInputWidget::goStart(UIContext &) {
		cursor = 0;
		cursorIterator = text.begin();
		cursorXOffset = 0;
	}

	void TextInputWidget::goEnd(UIContext &ui) {
		cursor = text.length();
		cursorIterator = text.end();
		cursorXOffset = ui.getRenderers().text.textWidth(text);
	}

	float TextInputWidget::getTextScale() const {
		return scale / 16;
	}
}
