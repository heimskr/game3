#include "entity/LivingTitledEntity.h"
#include "game/ClientGame.h"

namespace Game3 {
	void LivingTitledEntity::renderUpper(const RendererContext &renderers) {
		if (!isVisible()) {
			return;
		}

		LivingEntity::renderUpper(renderers);
		TitledEntity::renderUpper(renderers);
	}

	float LivingTitledEntity::getTitleVerticalOffset() const {
		return  canShowHealthBar()? -.5 : 0;
	}
}
