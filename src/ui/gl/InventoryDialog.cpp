#include "entity/Player.h"
#include "game/Inventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/InventoryDialog.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr double X_FRACTION = 0.25;
	constexpr double Y_FRACTION = 0.25;
}

namespace Game3 {
	InventoryDialog::InventoryDialog(PlayerPtr player):
		player(std::move(player)) {}

	void InventoryDialog::render(UIContext &ui, RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		Rectangle rectangle = stack.getTop();
		rectangle.x = rectangle.width * X_FRACTION / 2;
		rectangle.y = rectangle.height * Y_FRACTION / 2;
		rectangle.width *= (1. - X_FRACTION);
		rectangle.height *= (1. - Y_FRACTION);

		stack.pushRelative(rectangle);

		auto saver = renderers.getSaver();
		renderers.updateSize(rectangle.width, rectangle.height);

		double scale = 8;

		drawFrame(ui, renderers, scale, false, {
			"resources/gui/gui_topleft.png",
			"resources/gui/gui_top.png",
			"resources/gui/gui_topright.png",
			"resources/gui/gui_right.png",
			"resources/gui/gui_bottomright.png",
			"resources/gui/gui_bottom.png",
			"resources/gui/gui_bottomleft.png",
			"resources/gui/gui_left.png",
		}, Color{0.88, 0.77, 0.55});

		rectangle.x = 9 * scale;
		rectangle.y = 9 * scale;
		rectangle.width -= 18 * scale;
		rectangle.height -= 18 * scale;

		if (rectangle.height <= 0 || rectangle.width <= 0 || !player) {
			return;
		}

		stack.pushRelative(rectangle);

		renderers.updateSize(rectangle.width, rectangle.height);

		RectangleRenderer &rectangler = renderers.rectangle;

		rectangler.drawOnScreen(Color{0, 0, 0, 0.1}, 0, 0, 10000, 10000);

		constexpr static int INNER_SLOT_SIZE = 16;
		constexpr static int OUTER_SLOT_SIZE = INNER_SLOT_SIZE * 5 / 4;

		const int column_count = std::max(1, int(rectangle.width / (OUTER_SLOT_SIZE * scale)));
		const double x_pad = (rectangle.width - column_count * (OUTER_SLOT_SIZE * scale) + (OUTER_SLOT_SIZE - INNER_SLOT_SIZE) * scale) / scale / 2;

		int column = 0;
		double x = x_pad;
		double y = OUTER_SLOT_SIZE - INNER_SLOT_SIZE;

		InventoryPtr inventory = player->getInventory(0);
		auto inventory_lock = inventory->sharedLock();

		GamePtr game = player->getGame();

		SpriteRenderer &sprites = renderers.singleSprite;

		for (Slot slot = 0; slot < inventory->getSlotCount(); ++slot) {
			rectangler.drawOnScreen(Color{0, 0, 0, 0.1}, x * scale, y * scale, INNER_SLOT_SIZE * scale, INNER_SLOT_SIZE * scale);

			if (ItemStackPtr stack = (*inventory)[slot]) {
				ItemTexturePtr texture = stack->getTexture(*game);
				sprites.drawOnScreen(texture->getTexture(), RenderOptions{
					.x = x * scale,
					.y = y * scale,
					.offsetX = double(texture->x),
					.offsetY = double(texture->y),
					.sizeX = double(texture->width),
					.sizeY = double(texture->height),
					.scaleX = scale,
					.scaleY = scale,
					.invertY = false,
				});
			}

			x += OUTER_SLOT_SIZE;

			if (++column == column_count) {
				column = 0;
				x = x_pad;
				y += OUTER_SLOT_SIZE;
			}
		}
	}
}
