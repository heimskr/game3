#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "item/Item.h"
#include "packet/SetActiveSlotPacket.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	ItemSlot::ItemSlot(UIContext &ui, InventoryPtr inventory, ItemStackPtr stack, Slot slot, float size, float scale, bool active):
		Widget(ui, scale),
		inventory(std::move(inventory)),
		stack(std::move(stack)),
		slot(slot),
		size(size),
		active(active) {}

	ItemSlot::ItemSlot(UIContext &ui, Slot slot, float size, float scale, bool active):
		ItemSlot(ui, nullptr, nullptr, slot, size, scale, active) {}

	ItemSlot::ItemSlot(UIContext &ui, Slot slot, bool active):
		ItemSlot(ui, slot, INNER_SLOT_SIZE, SLOT_SCALE, active) {}

	void ItemSlot::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (width < 0)
			width = size * scale;

		if (height < 0)
			height = size * scale;

		adjustCoordinate(Orientation::Horizontal, x, width, size * scale);
		adjustCoordinate(Orientation::Vertical, y, height, size * scale);

		width = size * scale;
		height = size * scale;

		Widget::render(renderers, x, y, width, height);

		if (!ui.renderingDraggedWidget) {
			const float alpha = active? 0.4 : 0.15;
			renderers.rectangle.drawOnScreen(Color{0.6, 0.3, 0, alpha}, x, y, size * scale, size * scale);
		}

		if (!stack)
			return;

		std::shared_ptr<Tooltip> tooltip = ui.getTooltip();

		if (ui.checkMouse(lastRectangle)) {
			if (!tooltip->wasUpdatedBy(*this)) {
				if (tooltipText) {
					tooltip->setText(*tooltipText);
				} else {
					tooltip->setText(stack->getTooltip());
				}
			}
			tooltip->setRegion(lastRectangle);
			tooltip->show(*this);
		} else {
			tooltip->hide(*this);
		}

		if (!texture)
			texture = stack->getTexture(*ui.getGame());

		renderers.singleSprite.drawOnScreen(texture, RenderOptions{
			.x = x,
			.y = y,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale * 16. / texture->width,
			.scaleY = scale * 16. / texture->height,
			.invertY = false,
		});

		if (stack->count != static_cast<ItemCount>(-1)) {
			renderers.text.drawOnScreen(std::to_string(stack->count), TextRenderOptions{
				.x = x + (size - 3) * scale,
				.y = y + (size + 1) * scale,
				.scaleX = scale / 16,
				.scaleY = scale / 16,
				.shadowOffset{.375 * scale, .375 * scale},
			});
		}
	}

	std::shared_ptr<Widget> ItemSlot::getDragStartWidget() {
		return stack? shared_from_this() : nullptr;
	}

	bool ItemSlot::click(int, int, int) {
		if (inventory && inventory->getOwner() == ui.getPlayer())
			ui.getGame()->getPlayer()->send(make<SetActiveSlotPacket>(slot));
		return true;
	}

	bool ItemSlot::dragEnd(int, int) {
		if (!onDrop.empty()) {
			if (WidgetPtr dragged = ui.getDraggedWidget()) {
				onDrop(*this, dragged);
				return true;
			}
		}

		return false;
	}

	SizeRequestMode ItemSlot::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void ItemSlot::measure(const RendererContext &, Orientation, float, float, float &minimum, float &natural) {
		minimum = natural = size * scale;
	}

	void ItemSlot::setStack(ItemStackPtr new_stack) {
		stack = std::move(new_stack);
		texture = {};
	}

	ItemStackPtr ItemSlot::getStack() const {
		return stack;
	}

	void ItemSlot::setActive(bool new_active) {
		active = new_active;
	}

	Slot ItemSlot::getSlot() const {
		return slot;
	}

	std::shared_ptr<Inventory> ItemSlot::getInventory() const {
		return inventory;
	}

	void ItemSlot::setInventory(std::shared_ptr<Inventory> new_inventory) {
		inventory = std::move(new_inventory);
	}

	GlobalID ItemSlot::getOwnerGID() const {
		assert(inventory);
		AgentPtr owner = inventory->getOwner();
		assert(owner);
		return owner->getGID();
	}
}
