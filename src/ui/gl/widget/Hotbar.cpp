#include "Options.h"
#include "entity/ClientPlayer.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/gl/widget/Hotbar.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Hotbar::Hotbar(UIContext &ui, float scale): Widget(ui, scale) {
		for (Slot slot = 0; slot < HOTBAR_SIZE; ++slot) {
			slotWidgets.emplace_back(std::make_shared<ItemSlot>(ui, slot, INNER_SLOT_SIZE, scale, false));
		}
	}

	void Hotbar::render(const RendererContext &renderers, float x, float y, float width, float height) {
		const float original_width = width;
		const float original_height = height;
		float dummy{};

		measure(renderers, Orientation::Horizontal, original_width, original_height, dummy, width);
		measure(renderers, Orientation::Vertical,   original_width, original_height, dummy, height);
		Widget::render(renderers, x, y, width, height);

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
				const std::shared_ptr<ItemSlot> &widget = slotWidgets.at(slot);
				widget->setInventory(inventory);
				widget->setStack((*inventory)[slot]);
				widget->setActive(slot == active_slot);
				widget->render(renderers, x + scale * (SLOT_PADDING + OUTER_SLOT_SIZE * slot), y + scale * SLOT_PADDING, INNER_SLOT_SIZE * scale, INNER_SLOT_SIZE * scale);
			}
		}
	}

	bool Hotbar::click(int button, int x, int y) {
		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets)
			if (widget->contains(x, y) && widget->click(button, x, y))
				break;

		return true;
	}

	bool Hotbar::dragStart(int, int) {
		return true;
	}

	SizeRequestMode Hotbar::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Hotbar::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			minimum = natural = (OUTER_SLOT_SIZE * HOTBAR_SIZE + SLOT_PADDING) * scale + HOTBAR_BORDER * 2;
		} else {
			minimum = natural = (OUTER_SLOT_SIZE + SLOT_PADDING) * scale + HOTBAR_BORDER * 2;
		}
	}

	void Hotbar::reset() {
		for (const std::shared_ptr<ItemSlot> &item_slot: slotWidgets)
			item_slot->setStack(nullptr);
	}
}
