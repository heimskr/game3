#include "fluid/Fluid.h"

namespace Game3 {
	class Milk: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/milk"}; }

			Milk(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
