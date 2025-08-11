#include "entity/Entity.h"
#include "entity/SquareParticle.h"
#include "entity/Util.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	void spawnSquares(Entity &entity, size_t count, std::function<Color()> &&color_function, double linger_time) {
		RealmPtr realm = entity.getRealm();
		Position position = entity.getPosition();

		std::uniform_real_distribution y_distribution(-0.15, 0.15);
		std::uniform_real_distribution z_distribution(6., 10.);
		std::uniform_real_distribution depth_distribution(-0.5, 0.0);

		for (double x: {-1, +1}) {
			std::uniform_real_distribution x_distribution(x - 1.0, x + 1.0);
			for (size_t i = 0; i < count; ++i) {
				Vector3 velocity{
					x_distribution(threadContext.rng),
					y_distribution(threadContext.rng),
					z_distribution(threadContext.rng),
				};
				double depth = depth_distribution(threadContext.rng);
				realm->spawn<SquareParticle>(position, velocity, 0.2, color_function(), depth, linger_time)->offset.withUnique([](Vector3 &offset) {
					offset.y += 0.5;
				});
			}
		}
	}
}
