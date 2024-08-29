#include "graphics/BatchSpriteRenderer.h"
#include "graphics/CircleRenderer.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/Recolor.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	void RendererContext::pushSize() const {
		rectangle.pushBackbuffer();
		singleSprite.pushBackbuffer();
		batchSprite.pushBackbuffer();
		text.pushBackbuffer();
		circle.pushBackbuffer();
		recolor.pushBackbuffer();
	}

	void RendererContext::popSize() const {
		rectangle.popBackbuffer();
		singleSprite.popBackbuffer();
		batchSprite.popBackbuffer();
		text.popBackbuffer();
		circle.popBackbuffer();
		recolor.popBackbuffer();
	}

	void RendererContext::updateSize(int width, int height) const {
		rectangle.update(width, height);
		singleSprite.update(width, height);
		batchSprite.update(width, height);
		text.update(width, height);
		circle.update(width, height);
		recolor.update(width, height);
	}
}
