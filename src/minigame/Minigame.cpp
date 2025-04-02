#include "minigame/Minigame.h"
#include "ui/gl/Constants.h"

namespace Game3 {
	Minigame::Minigame(UIContext &ui, float selfScale):
		Widget(ui, selfScale) {}

	void Minigame::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			minimum = natural = gameWidth;
		} else {
			minimum = natural = gameHeight;
		}
	}

	void Minigame::setSize(int width, int height) {
		gameWidth = width;
		gameHeight = height;
	}

	void Minigame::onClose() {}

	SizeRequestMode Minigame::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Minigame::increaseScore(std::size_t addend) {
		score += addend;
	}
}
