#pragma once

namespace Game3 {
	class RectangleRenderer;
	class SpriteRenderer;
	class TextRenderer;

	struct RendererSet {
		RectangleRenderer &rectangle;
		SpriteRenderer &sprite;
		TextRenderer &text;

		RendererSet(RectangleRenderer &rectangle_, SpriteRenderer &sprite_, TextRenderer &text_):
			rectangle(rectangle_), sprite(sprite_), text(text_) {}
	};
}
