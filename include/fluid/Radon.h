#include "fluid/Fluid.h"

namespace Game3 {
	class Radon: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/radon"}; }

			Radon(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
