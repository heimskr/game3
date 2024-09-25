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

	BaseDraggableDialog::BaseDraggableDialog(UIContext &ui, int width, int height):
		Dialog(ui),
		position((ui.getWidth() - width) / 2, (ui.getHeight() - height) / 2, width, height) {}

	void BaseDraggableDialog::init() {
		closeButton = std::make_shared<Icon>(ui, UI_SCALE);
		closeButton->setIconTexture(cacheTexture("resources/gui/x.png"));
		closeButton->setFixedSize(3.5 * UI_SCALE);
		closeButton->init();
		closeButton->setOnClick([this](Widget &, int button, int, int) {
			INFO("closeButton clicked with mouse button {}", button);
			if (button != LEFT_BUTTON)
				return false;

			ui.removeDialog(shared_from_this());
			return true;
		});
	}

	void BaseDraggableDialog::render(const RendererContext &renderers) {
		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		constexpr auto scale = UI_SCALE;

		titleRectangle = position + Rectangle(scale, scale, position.width - 9 * scale, 7 * scale);

		rectangler.drawOnScreen(BORDER_COLOR, position);
		rectangler.drawOnScreen(TITLE_BACKGROUND_COLOR, titleRectangle);
		rectangler.drawOnScreen(INTERIOR_BACKGROUND_COLOR, position + Rectangle(scale, 9 * scale, position.width - 2 * scale, position.height - 10 * scale));

		closeButton->render(renderers, position + Rectangle(position.width - 5.75 * scale, 2.75 * scale, 3.5 * scale, 3.5 * scale));

		auto saver = ui.scissorStack.pushRelative(titleRectangle, renderers);
		const auto text_scale = static_cast<double>(getTitleScale());

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

	float DraggableDialog::getTitleScale() const {
		return UI_SCALE / 16;
	}

	const UString & DraggableDialog::getTitle() const {
		return title;
	}

	void DraggableDialog::setTitle(UString new_title) {
		title = std::move(new_title);
	}
}
