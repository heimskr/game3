#include "dialogue/RhosumShop.h"
#include "entity/ClientPlayer.h"
#include "entity/Speaker.h"
#include "game/ClientGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "item/Item.h"
#include "ui/widget/Icon.h"
#include "ui/widget/ItemSlot.h"
#include "ui/widget/Label.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	RhosumShopWidget::RhosumShopWidget(UIContext &ui, float selfScale, const std::shared_ptr<RhosumShopNode> &parent):
		FullscreenWidget(ui, selfScale),
		weakParent(parent) {}

	void RhosumShopWidget::init() {
		WidgetPtr self = getSelf();
		ClientGamePtr game = ui.getGame();

		talksprite = cacheTexture("resources/talksprites/rhosum_surprised.png");

		moneyLabel = make<Label>(ui, selfScale);
		moneyNoticer.setFunction([this](const MoneyCount *, const MoneyCount &money) {
			moneyLabel->setText(std::format("{} Q", money));
		});

		exitIcon = make<Icon>(ui, selfScale, "resources/gui/abscond.png");
		exitIcon->setFixedSize(14);
		exitIcon->setOnClick([this](Widget &) {
			auto parent = weakParent.lock();
			auto graph = parent->getParent();
			parent->selectOption(0);
		});
		exitIcon->insertAtEnd(self);

		lampOil = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/lamp_oil", -1), -1, INNER_SLOT_SIZE, 1);
		rope = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/rope", -1), -1, INNER_SLOT_SIZE, 1);
		bomb = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/bomb", -1), -1, INNER_SLOT_SIZE, 1);

		auto configure = [&](const ItemSlotPtr &item_slot) {
			item_slot->setOnClick([this](Widget &widget) {
				ItemSlotPtr item_slot = std::static_pointer_cast<ItemSlot>(widget.getSelf());
				ItemStackPtr stack = item_slot->getStack();
				bool success = ui.getPlayer()->removeMoney(stack->item->basePrice);
				weakParent.lock()->selectOption(success? 2 : 1);
			});
			item_slot->insertAtEnd(self);
		};

		configure(lampOil);
		configure(rope);
		configure(bomb);
	}

	void RhosumShopWidget::render(const RendererContext &renderers, float, float, float width, float height) {
		assert(talksprite != nullptr);

		moneyNoticer.update(ui.getPlayer()->getMoney());

		int window_width = ui.window.getWidth();
		int window_height = ui.window.getHeight();
		width = 90 * ui.scale;
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
			exitIcon->render(renderers, Rectangle(ui.scale * 68, ui.scale * 10, ui.scale * 14, ui.scale * 14));
		}

		moneyLabel->render(renderers, 0, 0, window_width, window_height);

		Widget::render(renderers, position.x, position.y, position.width, position.height);

		position.x = (window_width - face_width * ui.scale) / 2;
		position.y -= face_height * ui.scale;

		renderers.singleSprite.drawOnScreen(talksprite, RenderOptions::simple(position.x, position.y, ui.scale));
	}

	WidgetPtr RhosumShopNode::getWidget(UIContext &ui) {
		if (!cachedWidget) {
			cachedWidget = make<RhosumShopWidget>(ui, 1, std::static_pointer_cast<RhosumShopNode>(shared_from_this()));
		}

		return cachedWidget;
	}
}
