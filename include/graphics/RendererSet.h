#pragma once

namespace Game3 {
	class CircleRenderer;
	class RectangleRenderer;
	class SpriteRenderer;
	class TextRenderer;

	struct RendererSet {
		RectangleRenderer &rectangle;
		SpriteRenderer &sprite;
		TextRenderer &text;
		CircleRenderer &circle;

		RendererSet(RectangleRenderer &rectangle_, SpriteRenderer &sprite_, TextRenderer &text_, CircleRenderer &circle_):
			rectangle(rectangle_), sprite(sprite_), text(text_), circle(circle_) {}
	};
}
