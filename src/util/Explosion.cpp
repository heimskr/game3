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
			.particleType = "base:entity/explosion_particle",
			.radius = radius,
			.particleCount = particle_count,
			.destroysTileEntities = destroys_tile_entities,
		};

		causeExplosion(place, options);
	}
}
