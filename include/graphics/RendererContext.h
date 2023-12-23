#pragma once

namespace Game3 {
	class BatchSpriteRenderer;
	class CircleRenderer;
	class RectangleRenderer;
	class SingleSpriteRenderer;
	class TextRenderer;

	struct RendererContext {
		RectangleRenderer &rectangle;
		SingleSpriteRenderer &singleSprite;
		BatchSpriteRenderer &batchSprite;
		TextRenderer &text;
		CircleRenderer &circle;
		int factor;

		RendererContext(RectangleRenderer &rectangle_, SingleSpriteRenderer &single_sprite, BatchSpriteRenderer &batch_sprite, TextRenderer &text_, CircleRenderer &circle_, int factor_):
			rectangle(rectangle_), singleSprite(single_sprite), batchSprite(batch_sprite), text(text_), circle(circle_), factor(factor_) {}

		void pushSize();
		void popSize();
		void updateSize(int width, int height);

		class Saver {
			public:
				explicit Saver(RendererContext &context_): context(context_) {
					context.pushSize();
				}

				~Saver() {
					context.popSize();
				}

			private:
				RendererContext &context;
		};

		Saver getSaver() { return Saver(*this); }
	};
}
