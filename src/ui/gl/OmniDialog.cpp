#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "graphics/TextRenderer.h"
#include "packet/SwapSlotsPacket.h"
#include "ui/gl/ItemSlotWidget.h"
#include "ui/gl/OmniDialog.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr double X_FRACTION = 0.4;
	constexpr double Y_FRACTION = 0.4;
	constexpr double SCALE = 8;
	constexpr double UNSCALE = 1.5;
	constexpr int TOP_OFFSET = 20 * SCALE;
}

namespace Game3 {
	OmniDialog::OmniDialog(UIContext &ui, PlayerPtr player):
		Dialog(ui), player(std::move(player)) {}

	void OmniDialog::render(RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		auto saver = renderers.getSaver();

		Rectangle rectangle = getPosition();

		const Color inner_color{0.88, 0.77, 0.55};
		const std::array<std::string_view, 8> pieces{
			"resources/gui/gui_topleft.png",
			"resources/gui/gui_top.png",
			"resources/gui/gui_topright.png",
			"resources/gui/gui_right.png",
			"resources/gui/gui_bottomright.png",
			"resources/gui/gui_bottom.png",
			"resources/gui/gui_bottomleft.png",
			"resources/gui/gui_left.png",
		};


		Rectangle tab_rectangle = rectangle + Rectangle{0, int(rectangle.height - 6 * SCALE / UNSCALE), TOP_OFFSET, TOP_OFFSET};
		stack.pushRelative(tab_rectangle);
		renderers.updateSize(tab_rectangle.width, tab_rectangle.height);
		drawFrame(renderers, SCALE / UNSCALE, false, pieces, inner_color);
		stack.pop();

		stack.pushRelative(rectangle);
		renderers.updateSize(rectangle.width, rectangle.height);
		drawFrame(renderers, SCALE, false, pieces, inner_color);

		rectangle.x = 9 * SCALE;
		rectangle.y = 9 * SCALE;
		rectangle.width -= 18 * SCALE;
		rectangle.height -= 18 * SCALE;

		if (rectangle.height <= 0 || rectangle.width <= 0 || !player) {
			return;
		}

		innerRectangle = stack.pushRelative(rectangle);

		renderers.updateSize(rectangle.width, rectangle.height);

		RectangleRenderer &rectangler = renderers.rectangle;

		rectangler.drawOnScreen(Color{0, 0, 0, 0.1}, 0, 0, 10000, 10000);

		constexpr static int INNER_SLOT_SIZE = 16;
		constexpr static int OUTER_SLOT_SIZE = INNER_SLOT_SIZE * 5 / 4;

		const int column_count = std::max(1, int(rectangle.width / (OUTER_SLOT_SIZE * SCALE)));
		const double x_pad = (rectangle.width - column_count * (OUTER_SLOT_SIZE * SCALE) + (OUTER_SLOT_SIZE - INNER_SLOT_SIZE) * SCALE) / 2;

		int column = 0;
		double x = x_pad;
		double y = (OUTER_SLOT_SIZE - INNER_SLOT_SIZE) * SCALE;

		InventoryPtr inventory = player->getInventory(0);
		auto inventory_lock = inventory->sharedLock();

		const Slot slot_count = inventory->getSlotCount();
		assert(0 <= slot_count);

		const Slot active_slot = inventory->activeSlot;

		if (slotWidgets.size() != static_cast<size_t>(slot_count)) {
			slotWidgets.clear();
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets.emplace_back(std::make_shared<ItemSlotWidget>((*inventory)[slot], slot, INNER_SLOT_SIZE, SCALE, slot == active_slot));
		} else {
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets[slot]->setStack((*inventory)[slot]);

			if (0 <= previousActive) {
				if (previousActive != active_slot) {
					slotWidgets.at(previousActive)->setActive(false);
					slotWidgets.at(active_slot)->setActive(true);
				}
			} else {
				slotWidgets.at(active_slot)->setActive(true);
			}
		}

		previousActive = active_slot;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			widget->render(ui, renderers, x, y);

			x += OUTER_SLOT_SIZE * SCALE;

			if (++column == column_count) {
				column = 0;
				x = x_pad;
				y += OUTER_SLOT_SIZE * SCALE;
			}
		}
	}

	Rectangle OmniDialog::getPosition() const {
		Rectangle rectangle = ui.scissorStack.getBase();
		rectangle.x = rectangle.width * X_FRACTION / 2;
		rectangle.y = rectangle.height * Y_FRACTION / 2;
		rectangle.width *= (1. - X_FRACTION);
		rectangle.height *= (1. - Y_FRACTION);
		rectangle.height -= TOP_OFFSET;
		return rectangle;
	}

	bool OmniDialog::click(int x, int y) {
		if (!Dialog::click(x, y))
			return false;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y) && widget->click(ui))
				break;
		}

		return true;
	}

	bool OmniDialog::dragStart(int x, int y) {
		if (!Dialog::dragStart(x, y))
			return false;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ui.setDraggedWidget(widget->getDragStartWidget());
				break;
			}
		}

		return true;
	}

	bool OmniDialog::dragEnd(int x, int y) {
		if (!Dialog::dragEnd(x, y))
			return false;

		auto dragged = std::dynamic_pointer_cast<ItemSlotWidget>(ui.getDraggedWidget());

		if (!dragged)
			return true;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ClientPlayerPtr player = ui.getGame()->getPlayer();
				const InventoryID inventory_id = player->getInventory(0)->index;
				player->send(SwapSlotsPacket(player->getGID(), player->getGID(), dragged->getSlot(), widget->getSlot(), inventory_id, inventory_id));
				ui.setDraggedWidget(nullptr);
				break;
			}
		}

		return true;
	}
}
