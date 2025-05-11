#include "entity/Player.h"
#include "entity/SquareParticle.h"
#include "packet/ExplosionPacket.h"
#include "realm/Realm.h"
#include "types/Encodable.h"
#include "types/Position.h"
#include "util/Explosion.h"

namespace Game3 {
	void causeExplosion(const Place &place, const ExplosionOptions &options) {
		if (WeakSet<Player> players = place.realm->getPlayers().copyBase(); !players.empty()) {
			auto packet = make<ExplosionPacket>(place.realm->getID(), place.position, options);
			for (const std::weak_ptr<Player> &weak_player: players) {
				if (PlayerPtr player = weak_player.lock()) {
					player->send(packet);
				}
			}
		}
	}

	void causeExplosion(const Place &place, float radius, uint32_t particle_count, bool destroys_tile_entities) {
		ExplosionOptions options{
			.radius = radius,
			.particleCount = particle_count,
			.destroysTileEntities = destroys_tile_entities,
		};

		options << SquareParticle::RandomizationOptions{
			.sizeMin = 0.2,
			.sizeMax = 0.4,
			.valueMin = 0.3,
			.valueMax = 0.7,
		};

		causeExplosion(place, options);
	}
}
