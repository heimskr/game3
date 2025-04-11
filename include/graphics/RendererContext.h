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
		float delta;

		RendererContext(RectangleRenderer &rectangle, SingleSpriteRenderer &singleSprite, BatchSpriteRenderer &batchSprite, TextRenderer &text, CircleRenderer &circle, Recolor &recolor, Lockable<ClientSettings> &settings, float xFactor, float yFactor, float delta):
			rectangle(rectangle),
			singleSprite(singleSprite),
			batchSprite(batchSprite),
			text(text),
			circle(circle),
			recolor(recolor),
			settings(settings),
			xFactor(xFactor),
			yFactor(yFactor),
			delta(delta) {}

		void pushSize() const;
		void popSize() const;
		void updateSize(int width, int height) const;

		SizeSaver getSaver() const { return SizeSaver(*this); }
	};
}
