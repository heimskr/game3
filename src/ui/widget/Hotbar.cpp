#include "Options.h"
#include "entity/ClientPlayer.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "packet/MoveSlotsPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "ui/widget/Hotbar.h"
#include "ui/widget/ItemSlot.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"

namespace Game3 {
	Hotbar::Hotbar(UIContext &ui, float selfScale):
		Widget(ui, selfScale) {}

	void Hotbar::init() {
		WidgetPtr self = shared_from_this();

		for (Slot slot = 0; slot < HOTBAR_SIZE; ++slot) {
			auto &slot_widget = slotWidgets.emplace_back(make<ItemSlot>(ui, slot, INNER_SLOT_SIZE, selfScale, false));
			slot_widget->onDrop.connect([this](ItemSlot &self, const WidgetPtr &widget) {
				if (auto dragged = std::dynamic_pointer_cast<ItemSlot>(widget); dragged && dragged.get() != &self) {
					ClientPlayerPtr player = ui.getPlayer();
					player->send(make<MoveSlotsPacket>(dragged->getOwnerGID(), self.getOwnerGID(), dragged->getSlot(), self.getSlot(), dragged->getInventory()->index, self.getInventory()->index));
				}
			});
			slot_widget->insertAtEnd(self);
		}

		heldLeft  = make<ItemSlot>(ui, -1, INNER_SLOT_SIZE, selfScale / 2, false);
		heldRight = make<ItemSlot>(ui, -1, INNER_SLOT_SIZE, selfScale / 2, false);

		heldLeft->insertAtEnd(self);
		heldRight->insertAtEnd(self);

		auto make_held_drop = [this](bool is_left) {
			return [this, is_left](ItemSlot &self, const WidgetPtr &widget) {
				if (auto dragged = std::dynamic_pointer_cast<ItemSlot>(widget)) {
					if (*dragged->getInventory() != *self.getInventory()) {
						return;
					}

					if (Slot slot = dragged->getSlot(); slot >= 0) {
						ui.getPlayer()->send(make<SetHeldItemPacket>(is_left, slot));
					}
				}
			};
		};

		heldLeft->onDrop.connect(make_held_drop(true));
		heldRight->onDrop.connect(make_held_drop(false));

		auto make_held_click = [this](bool is_left) {
			return [this, is_left](Widget &) {
				ui.getPlayer()->send(make<SetHeldItemPacket>(is_left, -1));
			};
		};

		heldLeft->setOnClick(make_held_click(true));
		heldRight->setOnClick(make_held_click(false));
	}

	void Hotbar::render(const RendererContext &renderers, float x, float y, float width, float height) {
		const float original_width = width;
		const float original_height = height;
		float dummy{};

		measure(renderers, Orientation::Horizontal, original_width, original_height, dummy, width);
		measure(renderers, Orientation::Vertical,   original_width, original_height, dummy, height);
		Widget::render(renderers, x, y, width, height);
		width -= getRightSideWidth();
		const auto scale = getScale();
		const float offset = SLOT_PADDING * scale / 3;
		const float held_scale = heldLeft->getScale();

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		constexpr Color outer_color{0.7, 0.5, 0, 1};
		constexpr Color inner_color{0.88, 0.77, 0.55, 1};
		rectangler.drawOnScreen(outer_color, x, y, width, height);
		x += offset;
		y += offset;
		width -= offset * 2;
		height -= offset * 2;
		rectangler.drawOnScreen(inner_color, x, y, width, height);
		lastY = y;

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

			heldLeft->setInventory(inventory);
			heldRight->setInventory(inventory);

			Slot left_slot  = player->getHeldLeft();
			Slot right_slot = player->getHeldRight();

			if (left_slot >= 0) {
				heldLeft->setStack((*inventory)[left_slot]);
			} else {
				heldLeft->setStack(nullptr);
			}

			if (right_slot >= 0) {
				heldRight->setStack((*inventory)[right_slot]);
			} else {
				heldRight->setStack(nullptr);
			}

			const float held_size = INNER_SLOT_SIZE * held_scale;
			const float held_padding = SLOT_PADDING * held_scale;
			const float text_scale = held_scale / 8;
			const float rectangle_size = held_size + 2 * held_padding;

			x += width + 2 * offset;
			rectangler.drawOnScreen(outer_color, x,          y - offset, rectangle_size + 2 * offset, height + offset * 2);
			rectangler.drawOnScreen(inner_color, x + offset, y,          rectangle_size,              height);

			x += offset + held_padding;
			y += held_padding;

			TextRenderOptions options{
				.x = x + held_padding + held_size / 4, // Idk honestly. This is what makes it look nice.
				.y = y + held_padding,
				.scaleX = text_scale,
				.scaleY = text_scale,
				.color = Color{"#00000080"},
				.align = TextAlign::Center,
				.alignTop = true,
				.shadow{0, 0, 0, 0},
			};

			if (left_slot < 0) {
				texter.drawOnScreen("L", options);
			}

			if (right_slot < 0) {
				options.y += 2 * held_padding + held_size;
				texter.drawOnScreen("R", options);
			}

			heldLeft ->render(renderers, x, y, held_size, held_size);
			y += 2 * held_padding + held_size;
			heldRight->render(renderers, x, y, held_size, held_size);
		}
	}

	bool Hotbar::blocksMouse(int x, int y, bool is_drag_update) const {
		return !is_drag_update && contains(x, y);
	}

	SizeRequestMode Hotbar::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Hotbar::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		const auto scale = getScale();
		if (orientation == Orientation::Horizontal) {
			minimum = natural = (OUTER_SLOT_SIZE * HOTBAR_SIZE + SLOT_PADDING + HOTBAR_BORDER * 2) * scale + getRightSideWidth();
		} else {
			minimum = natural = (OUTER_SLOT_SIZE + SLOT_PADDING + HOTBAR_BORDER * 2) * scale;
		}
	}

	void Hotbar::reset() {
		for (const std::shared_ptr<ItemSlot> &item_slot: slotWidgets) {
			item_slot->setStack(nullptr);
		}
	}

	const std::optional<float> & Hotbar::getLastY() const {
		return lastY;
	}

	float Hotbar::getRightSideWidth() const {
		const auto scale = getScale();
		const auto offset = SLOT_PADDING * scale / 3;
		const auto held_scale = heldLeft->getScale();
		const auto held_size = INNER_SLOT_SIZE * held_scale;
		const auto held_padding = SLOT_PADDING * held_scale;
		const auto rectangle_size = held_size + 2 * held_padding + 2 * offset;
		return 2 * offset + rectangle_size;
	}
}
