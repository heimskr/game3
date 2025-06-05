#pragma once

#include "entity/LivingEntity.h"
#include "entity/TitledEntity.h"

namespace Game3 {
	class LivingTitledEntity: public LivingEntity, public TitledEntity {
		public:
			void renderUpper(const RendererContext &) override;

			float getTitleVerticalOffset() const override;
	};
}
