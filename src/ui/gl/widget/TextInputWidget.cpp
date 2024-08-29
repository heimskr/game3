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
}

namespace Game3 {
	TextInputWidget::TextInputWidget(float scale, Color exterior_color, Color interior_color, Color text_color, float thickness):
		scale(scale), thickness(thickness), exteriorColor(exterior_color), interiorColor(interior_color), textColor(text_color) {
			text = "This is a test";
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

	bool TextInputWidget::keyPressed(UIContext &, uint32_t character) {
		switch (character) {
			case GDK_KEY_Return:
				character = '\n';
				break;

			case GDK_KEY_BackSpace:
				if (!text.empty())
					text.erase(text.size() - 1, 1);
				return true;

			default:
				break;
		}

		text += static_cast<gunichar>(character);
		return true;
	}

	float TextInputWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return available_height;
	}
}
