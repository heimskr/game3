#include "dialogue/RhosumShop.h"
#include "game/ClientGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "item/Item.h"
#include "ui/widget/ItemSlot.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	RhosumShopWidget::RhosumShopWidget(UIContext &ui, float selfScale):
		FullscreenWidget(ui, selfScale) {}

	void RhosumShopWidget::init() {
		WidgetPtr self = getSelf();
		ClientGamePtr game = ui.getGame();

		talksprite = cacheTexture("resources/talksprites/rhosum_surprised.png");

		lampOil = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/lamp_oil"), -1, INNER_SLOT_SIZE, 1);
		rope = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/rope"), -1, INNER_SLOT_SIZE, 1);
		bomb = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/bomb"), -1, INNER_SLOT_SIZE, 1);

		lampOil->insertAtEnd(self);
		rope->insertAtEnd(self);
		bomb->insertAtEnd(self);
	}

	void RhosumShopWidget::render(const RendererContext &renderers, float x, float y, float width, float height) {
		assert(talksprite != nullptr);

		int window_width = ui.window.getWidth();
		int window_height = ui.window.getHeight();
		width = 74 * ui.scale;
		height = 34 * ui.scale;
		int face_width = talksprite->width;
		int face_height = talksprite->height;

		Rectangle position((window_width - width) / 2, (window_height - height + face_height * ui.scale) / 2, width, height);

		{
			auto saver = ui.scissorStack.pushAbsolute(position, renderers);
			ui.drawFrame(renderers, ui.scale, false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);

			lampOil->render(renderers, Rectangle(ui.scale * 9, ui.scale * 9, -1, -1));
			rope->render(renderers, Rectangle(ui.scale * 29, ui.scale * 9, -1, -1));
			bomb->render(renderers, Rectangle(ui.scale * 49, ui.scale * 9, -1, -1));
		}

		Widget::render(renderers, position.x, position.y, position.width, position.height);

		position.x = (window_width - face_width * ui.scale) / 2;
		position.y -= face_height * ui.scale;

		renderers.singleSprite.drawOnScreen(talksprite, RenderOptions::simple(position.x, position.y, ui.scale));
	}

	WidgetPtr RhosumShopNode::getWidget(UIContext &ui) {
		if (!cachedWidget) {
			cachedWidget = make<RhosumShopWidget>(ui, 1);
		}

		return cachedWidget;
	}
}
