#include "fluid/Fluid.h"

namespace Game3 {
	class Water: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/water"}; }

			Water(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
