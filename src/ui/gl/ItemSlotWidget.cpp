#include "game/ClientGame.h"
#include "graphics/ItemTexture.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "item/Item.h"
#include "ui/gl/ItemSlotWidget.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	ItemSlotWidget::ItemSlotWidget(ItemStackPtr stack, double size, double scale, bool active):
		stack(std::move(stack)), size(size), scale(scale), active(active) {}

	void ItemSlotWidget::render(UIContext &ui, RendererContext &renderers, float x, float y) {
		Widget::render(ui, renderers, x, y);

		const float alpha = active? 0.4 : 0.1;

		renderers.rectangle.drawOnScreen(Color{0, 0, 0, alpha}, x * scale, y * scale, size * scale, size * scale);

		if (!stack)
			return;

		if (!texture)
			texture = stack->getTexture(*ui.getGame());

		renderers.singleSprite.drawOnScreen(texture->getTexture(), RenderOptions{
			.x = x * scale,
			.y = y * scale,
			.offsetX = double(texture->x),
			.offsetY = double(texture->y),
			.sizeX = double(texture->width),
			.sizeY = double(texture->height),
			.scaleX = scale * 16. / texture->width,
			.scaleY = scale * 16. / texture->height,
			.invertY = false,
		});

		renderers.text.drawOnScreen(std::to_string(stack->count), TextRenderOptions{
			.x = (x + size - 3) * scale,
			.y = (y + size + 1) * scale,
			.scaleX = scale / 16,
			.scaleY = scale / 16,
			.shadowOffset{.375 * scale, .375 * scale},
		});
	}

	void ItemSlotWidget::setStack(std::shared_ptr<ItemStack> new_stack) {
		stack = std::move(new_stack);
		texture = {};
	}

	void ItemSlotWidget::setActive(bool new_active) {
		active = new_active;
	}
}
