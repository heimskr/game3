#include "item/Drink.h"

namespace Game3 {
	class Mead: public Drink<"base:fluid/mead"> {
		public:
			using Drink<"base:fluid/mead">::Drink;

			HitPoints getHealedPoints(const std::shared_ptr<Player> &player) override;
	};
}
