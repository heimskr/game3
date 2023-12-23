#include "graphics/BatchSpriteRenderer.h"
#include "graphics/CircleRenderer.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	void RendererContext::pushSize() {
		rectangle.pushBackbuffer();
		singleSprite.pushBackbuffer();
		batchSprite.pushBackbuffer();
		text.pushBackbuffer();
		circle.pushBackbuffer();
	}

	void RendererContext::popSize() {
		rectangle.popBackbuffer();
		singleSprite.popBackbuffer();
		batchSprite.popBackbuffer();
		text.popBackbuffer();
		circle.popBackbuffer();
	}

	void RendererContext::updateSize(int width, int height) {
		rectangle.update(width, height);
		singleSprite.update(width, height);
		batchSprite.update(width, height);
		text.update(width, height);
		circle.update(width, height);
	}
}
