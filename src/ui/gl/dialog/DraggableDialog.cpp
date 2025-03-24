#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	namespace {
		constexpr Color BORDER_COLOR{"#7f5832"};
		constexpr Color TITLE_BACKGROUND_COLOR{"#cc9a65"};
		constexpr Color TITLE_TEXT_COLOR{"#000000"};
		constexpr Color INTERIOR_BACKGROUND_COLOR{"#ffc07e"};
	}

	int BaseDraggableDialog::getEffectiveWidth(int content_width, float scale) {
		return content_width + 4 * scale;
	}

	int BaseDraggableDialog::getEffectiveHeight(int content_height, float scale) {
		return content_height + 12 * scale;
	}

	BaseDraggableDialog::BaseDraggableDialog(UIContext &ui, float selfScale, int width, int height):
		Dialog(ui, selfScale),
		position((ui.getWidth() - width) / 2, (ui.getHeight() - height) / 2, width, height) {}

	void BaseDraggableDialog::render(const RendererContext &renderers) {
		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const auto scale = getScale();

		titleRectangle = position + Rectangle(scale, scale, position.width - 9 * scale, 7 * scale);
		bodyRectangle = position + Rectangle(scale, 9 * scale, position.width - 2 * scale, position.height - 10 * scale);

		rectangler.drawOnScreen(BORDER_COLOR, position);
		rectangler.drawOnScreen(TITLE_BACKGROUND_COLOR, titleRectangle);

		bodyRectangle = getInnerRectangle();

		Rectangle outer_rectangle = bodyRectangle;
		outer_rectangle.x -= scale;
		outer_rectangle.y -= scale;
		outer_rectangle.width += 2 * scale;
		outer_rectangle.height += 2 * scale;
		rectangler.drawOnScreen(INTERIOR_BACKGROUND_COLOR, outer_rectangle);

		closeButton->render(renderers, position + Rectangle(position.width - 5.75 * scale, 3 * scale, 3.5 * scale, 4 * scale));

		auto saver = ui.scissorStack.pushRelative(titleRectangle, renderers);
		const auto text_scale = static_cast<double>(getTitleScale()) * ui.scale;

		texter.drawOnScreen(getTitle(), TextRenderOptions{
			.x = static_cast<double>(titleRectangle.width) / 2.0,
			.y = static_cast<double>(scale) * 1.5,
			.scaleX = text_scale,
			.scaleY = text_scale,
			.wrapWidth = static_cast<double>(titleRectangle.width),
			.color = TITLE_TEXT_COLOR,
			.align = TextAlign::Center,
			.alignTop = true,
			.shadow{0, 0, 0, 0},
		});
	}

	Rectangle BaseDraggableDialog::getPosition() const {
		return position;
	}

	Rectangle BaseDraggableDialog::getInnerRectangle() const {
		const auto scale = getScale();
		return getPosition() + Rectangle(2 * scale, 10 * scale, position.width - 4 * scale, position.height - 12 * scale);
	}

	void BaseDraggableDialog::init() {
		closeButton = std::make_shared<Icon>(ui, selfScale);
		closeButton->setIconTexture(cacheTexture("resources/gui/x.png"));
		closeButton->setFixedSize(3.5 * selfScale);
		closeButton->init();
		closeButton->setOnClick([this](Widget &, int button, int, int) {
			if (button != LEFT_BUTTON)
				return false;

			signalDismiss();
			ui.removeDialog(getSelf());
			return true;
		});
	}

	bool BaseDraggableDialog::click(int button, int x, int y, Modifiers modifiers) {
		if (closeButton->contains(x, y)) {
			return closeButton->click(button, x, y, modifiers);
		}

		return Dialog::click(button, x, y, modifiers);
	}

	bool BaseDraggableDialog::dragStart(int x, int y) {
		if (titleRectangle.contains(x, y)) {
			dragOffset.emplace(x - position.x, y - position.y);
			return true;
		}

		return Dialog::dragStart(x, y);
	}

	bool BaseDraggableDialog::dragUpdate(int x, int y) {
		if (dragOffset) {
			position.x = x - dragOffset->first;
			position.y = y - dragOffset->second;
			return true;
		}

		return Dialog::dragUpdate(x, y);
	}

	bool BaseDraggableDialog::dragEnd(int x, int y) {
		dragOffset.reset();
		return Dialog::dragEnd(x, y);
	}

	bool BaseDraggableDialog::keyPressed(uint32_t key, Modifiers, bool) {
		if (key == GLFW_KEY_ESCAPE) {
			signalDismiss();
			ui.removeDialog(getSelf());
			return true;
		}

		return false;
	}

	float DraggableDialog::getTitleScale() const {
		return selfScale / 16;
	}

	const UString & DraggableDialog::getTitle() const {
		return title;
	}

	void DraggableDialog::setTitle(UString new_title) {
		title = std::move(new_title);
	}
}
