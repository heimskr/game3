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
	TextInputWidget::TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, float thickness):
		scale(scale), thickness(thickness), exteriorColor(exterior_color), interiorColor(interior_color), textColor(text_color) {
			text = "This is a test";
			cursor = text.end();
			xOffset = 0;
		}

	TextInputWidget::TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color):
		TextInputWidget(scale, exterior_color, interior_color, text_color, DEFAULT_THICKNESS) {}

	TextInputWidget::TextInputWidget(float scale, float thickness):
		TextInputWidget(scale, DEFAULT_EXTERIOR_COLOR, DEFAULT_INTERIOR_COLOR, DEFAULT_TEXT_COLOR, thickness) {}

	TextInputWidget::TextInputWidget(float scale):
		TextInputWidget(scale, DEFAULT_THICKNESS) {}

	void TextInputWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const float offset = thickness * scale;
		// TODO: check for negative sizes
		const Rectangle interior(x + offset, y + offset, width - 2 * offset, height - 2 * offset);

		rectangler(exteriorColor, x, y, width, height);
		rectangler(interiorColor, interior);

		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		texter(text, TextRenderOptions{
			.x = x - xOffset * scale + offset,
			.y = y,
			.scaleX = scale / 16,
			.scaleY = scale / 16,
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
				if (text.empty() || cursor == text.begin())
					return true;

				if (modifiers.onlyCtrl()) {
					eraseWord();
				} else {
					eraseCharacter();
				}

				return true;

			case GDK_KEY_Delete:
				if (!text.empty() && cursor != text.end())
					cursor = text.erase(cursor);
				return true;

			case GDK_KEY_Left:
				if (cursor != text.begin())
					--cursor;
				return true;

			case GDK_KEY_Right:
				if (cursor != text.end())
					++cursor;
				return true;

			case GDK_KEY_Home:
				cursor = text.begin();
				return true;

			case GDK_KEY_End:
				cursor = text.end();
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

		cursor = text.insert(cursor, static_cast<gunichar>(character));
		++cursor;
		return true;
	}

	float TextInputWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return available_height;
	}

	void TextInputWidget::eraseWord() {
		// TODO: instead of erasing multiple times, search the string for how much to erase and erase it all in one go.
		if (isStopChar(cursor)) {
			do {
				eraseCharacter();
			} while (cursor != text.begin() && isStopChar(cursor));
		} else {
			do {
				eraseCharacter();
			} while (cursor != text.begin() && !isStopChar(cursor));
			while (cursor != text.begin() && isWhitespace(cursor)) {
				eraseCharacter();
			}
		}
	}

	void TextInputWidget::eraseCharacter() {
		cursor = text.erase(--cursor);
	}
}
