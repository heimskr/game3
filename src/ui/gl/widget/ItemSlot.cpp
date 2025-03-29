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
#include "packet/UseItemPacket.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	ItemSlot::ItemSlot(UIContext &ui, InventoryPtr inventory, ItemStackPtr stack, Slot slot, float size, float selfScale, bool active):
		Widget(ui, selfScale),
		inventory(std::move(inventory)),
		stack(std::move(stack)),
		slot(slot),
		size(size),
		active(active) {}

	ItemSlot::ItemSlot(UIContext &ui, Slot slot, float size, float selfScale, bool active):
		ItemSlot(ui, nullptr, nullptr, slot, size, selfScale, active) {}

	ItemSlot::ItemSlot(UIContext &ui, Slot slot, bool active):
		ItemSlot(ui, slot, INNER_SLOT_SIZE, SLOT_SCALE, active) {}

	void ItemSlot::render(const RendererContext &renderers, float x, float y, float width, float height) {
		const auto scale = getScale();

		if (width < 0) {
			width = size * scale;
		}

		if (height < 0) {
			height = size * scale;
		}

		adjustCoordinate(Orientation::Horizontal, x, width, size * scale);
		adjustCoordinate(Orientation::Vertical, y, height, size * scale);

		Widget::render(renderers, x, y, width, height);

		if (!ui.renderingDraggedWidget) {
			const float alpha = active? 0.4 : 0.15;
			renderers.rectangle.drawOnScreen(Color{0.6, 0.3, 0, alpha}, x, y, size * scale, size * scale);
		}

		if (!stack) {
			return;
		}

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

		if (!texture) {
			texture = stack->getTexture(*ui.getGame());
		}

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

	bool ItemSlot::click(int button, int x, int y, Modifiers modifiers) {
		if (button == LEFT_BUTTON && slot >= 0) {
			if (inventory && inventory->getOwner() == ui.getPlayer()) {
				if (modifiers.onlyCtrl()) {
					if (stack) {
						ui.getGame()->getPlayer()->send(make<UseItemPacket>(slot, modifiers));
					}
				} else {
					ui.getGame()->getPlayer()->send(make<SetActiveSlotPacket>(slot));
				}

				return true;
			}
		}

		return Widget::click(button, x, y, modifiers);
	}

	bool ItemSlot::dragStart(int x, int y) {
		WidgetPtr dragged_widget = getDragStartWidget();
		const bool out = dragged_widget != nullptr;
		ui.setDraggedWidget(std::move(dragged_widget));
		return Widget::dragStart(x, y) || out;
	}

	bool ItemSlot::dragEnd(int x, int y) {
		if (!onDrop.empty()) {
			if (WidgetPtr dragged = ui.getDraggedWidget()) {
				if (dragged.get() == this) {
					return false;
				}

				onDrop(*this, dragged);
				return true;
			}
		}

		return Widget::dragEnd(x, y);
	}

	SizeRequestMode ItemSlot::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void ItemSlot::measure(const RendererContext &, Orientation, float, float, float &minimum, float &natural) {
		minimum = natural = size * getScale();
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
