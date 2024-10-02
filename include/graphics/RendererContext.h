#pragma once

#include "client/ClientSettings.h"
#include "graphics/SizeSaver.h"
#include "threading/Lockable.h"

namespace Game3 {
	class BatchSpriteRenderer;
	class CircleRenderer;
	class RectangleRenderer;
	class Recolor;
	class SingleSpriteRenderer;
	class TextRenderer;
	struct ClientSettings;

	struct RendererContext {
		RectangleRenderer &rectangle;
		SingleSpriteRenderer &singleSprite;
		BatchSpriteRenderer &batchSprite;
		TextRenderer &text;
		CircleRenderer &circle;
		Recolor &recolor;
		Lockable<ClientSettings> &settings;
		float xFactor;
		float yFactor;

		RendererContext(RectangleRenderer &rectangle, SingleSpriteRenderer &single_sprite, BatchSpriteRenderer &batch_sprite, TextRenderer &text, CircleRenderer &circle, Recolor &recolor, Lockable<ClientSettings> &settings, float x_factor, float y_factor):
			rectangle(rectangle), singleSprite(single_sprite), batchSprite(batch_sprite), text(text), circle(circle), recolor(recolor), settings(settings), xFactor(x_factor), yFactor(y_factor) {}

		void pushSize() const;
		void popSize() const;
		void updateSize(int width, int height) const;

		SizeSaver getSaver() const { return SizeSaver(*this); }
	};
}
