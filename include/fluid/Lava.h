#include "fluid/Fluid.h"

namespace Game3 {
	class Lava: public Fluid {
		public:
			Lava(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
