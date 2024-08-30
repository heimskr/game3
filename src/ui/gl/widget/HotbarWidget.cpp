#include "Options.h"
#include "entity/ClientPlayer.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/gl/widget/HotbarWidget.h"
#include "ui/gl/widget/ItemSlotWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	HotbarWidget::HotbarWidget(float scale): Widget(scale) {
		for (Slot slot = 0; slot < HOTBAR_SIZE; ++slot) {
			slotWidgets.emplace_back(std::make_shared<ItemSlotWidget>(slot, INNER_SLOT_SIZE, scale, false));
		}
	}

	void HotbarWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		const float offset = SLOT_PADDING * scale / 3;
		renderers.rectangle.drawOnScreen(Color{0.7, 0.5, 0, 1}, x, y, width, height);
		x += offset;
		y += offset;
		width -= offset * 2;
		height -= offset * 2;
		renderers.rectangle.drawOnScreen(Color{0.88, 0.77, 0.55, 1}, x, y, width, height);

		PlayerPtr player = ui.getPlayer();

		if (player) {
			InventoryPtr inventory = player->getInventory(0);
			assert(inventory);
			auto lock = inventory->sharedLock();
			Slot active_slot = inventory->activeSlot;

			for (Slot slot = 0; slot < HOTBAR_SIZE; ++slot) {
				const std::shared_ptr<ItemSlotWidget> &widget = slotWidgets.at(slot);
				widget->setInventory(inventory);
				widget->setStack((*inventory)[slot]);
				widget->setActive(slot == active_slot);
				widget->render(ui, renderers, x + scale * (SLOT_PADDING + OUTER_SLOT_SIZE * slot), y + scale * SLOT_PADDING, INNER_SLOT_SIZE * scale, INNER_SLOT_SIZE * scale);
			}
		}
	}

	bool HotbarWidget::click(UIContext &ui, int button, int x, int y) {
		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets)
			if (widget->getLastRectangle().contains(x, y) && widget->click(ui, button, x, y))
				break;

		return true;
	}

	bool HotbarWidget::dragStart(UIContext &, int, int) {
		return true;
	}

	float HotbarWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return available_height;
	}
}
