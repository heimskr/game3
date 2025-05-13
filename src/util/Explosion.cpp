#include "entity/Player.h"
#include "entity/SquareParticle.h"
#include "math/FilledCircle.h"
#include "packet/ExplosionPacket.h"
#include "realm/Realm.h"
#include "types/Encodable.h"
#include "types/Position.h"
#include "util/Explosion.h"

namespace Game3 {
	void causeExplosion(const Place &place, const ExplosionOptions &options) {
		assert(place.realm->getSide() == Side::Server);

		if (WeakSet<Player> players = place.realm->getPlayers().copyBase(); !players.empty()) {
			auto packet = make<ExplosionPacket>(place.realm->getID(), place.position, options);
			for (const std::weak_ptr<Player> &weak_player: players) {
				if (PlayerPtr player = weak_player.lock()) {
					player->send(packet);
				}
			}
		}

		iterateFilledCircle<Position::IntType>(place.position.column, place.position.row, options.radius, [&](auto x, auto y) {
			Position position{y, x};
			place.realm->damageGround(position);
			if (options.destroysTileEntities) {
				if (TileEntityPtr tile = place.realm->tileEntityAt(position); tile && tile->kill()) {
					tile->destroy();
				}
			}
		});
	}

	void causeExplosion(const Place &place, float radius, bool destroys_tile_entities) {
		ExplosionOptions options{
			.particleType = "base:entity/explosion_particle",
			.radius = radius,
			.destroysTileEntities = destroys_tile_entities,
		};

		causeExplosion(place, options);
	}
}
