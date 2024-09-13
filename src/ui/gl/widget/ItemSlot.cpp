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
#include "ui/gl/UIContext.h"

namespace Game3 {
	ItemSlot::ItemSlot(InventoryPtr inventory, ItemStackPtr stack, Slot slot, float size, float scale, bool active):
		Widget(scale),
		inventory(std::move(inventory)),
		stack(std::move(stack)),
		slot(slot),
		size(size),
		active(active) {}

	ItemSlot::ItemSlot(Slot slot, float size, float scale, bool active):
		ItemSlot(nullptr, nullptr, slot, size, scale, active) {}

	void ItemSlot::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		if (!ui.renderingDraggedWidget) {
			const float alpha = active? 0.4 : 0.15;
			renderers.rectangle.drawOnScreen(Color{0.6, 0.3, 0, alpha}, x, y, size * scale, size * scale);
		}

		if (!stack)
			return;

		std::shared_ptr<Tooltip> tooltip = ui.getTooltip();

		if (ui.checkMouseAbsolute(lastRectangle)) {
			if (!tooltip->wasUpdatedBy(*this)) {
				tooltip->setText(stack->getTooltip());
				tooltip->setRegion(lastRectangle);
				tooltip->show(*this);
			}
		} else {
			tooltip->hide(*this);
		}

		if (!texture)
			texture = stack->getTexture(*ui.getGame());

		renderers.singleSprite.drawOnScreen(texture->getTexture(), RenderOptions{
			.x = x,
			.y = y,
			.offsetX = double(texture->x),
			.offsetY = double(texture->y),
			.sizeX = double(texture->width),
			.sizeY = double(texture->height),
			.scaleX = scale * 16. / texture->width,
			.scaleY = scale * 16. / texture->height,
			.invertY = false,
		});

		renderers.text.drawOnScreen(std::to_string(stack->count), TextRenderOptions{
			.x = x + (size - 3) * scale,
			.y = y + (size + 1) * scale,
			.scaleX = scale / 16,
			.scaleY = scale / 16,
			.shadowOffset{.375 * scale, .375 * scale},
		});
	}

	std::shared_ptr<Widget> ItemSlot::getDragStartWidget() {
		return stack? shared_from_this() : nullptr;
	}

	bool ItemSlot::click(UIContext &ui, int, int, int) {
		if (inventory && inventory->getOwner() == ui.getPlayer())
			ui.getGame()->getPlayer()->send(SetActiveSlotPacket(slot));
		return true;
	}

	SizeRequestMode ItemSlot::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void ItemSlot::measure(const RendererContext &, Orientation, float, float, float &minimum, float &natural) {
		minimum = natural = 16 * scale;
	}

	void ItemSlot::setStack(std::shared_ptr<ItemStack> new_stack) {
		stack = std::move(new_stack);
		texture = {};
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
