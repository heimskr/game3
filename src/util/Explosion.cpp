#include "entity/Player.h"
#include "entity/ServerPlayer.h"
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

		const Vector2d origin(place.position);

		iterateFilledCircle<Position::IntType>(place.position.column, place.position.row, options.radius, [&, &realm = *place.realm, origin, damage_scale = options.damageScale, radius = options.radius](auto x, auto y) {
			Position position{y, x};

			place.realm->damageGround(position);

			if (options.damageScale != 0) {
				for (const EntityPtr &hit_entity: realm.findEntities(position)) {
					if (LivingEntityPtr living_entity = std::dynamic_pointer_cast<LivingEntity>(hit_entity)) {
						Vector2d entity_vector = Vector2d(living_entity->getPosition()) + living_entity->getOffset();
						float distance = entity_vector.distance(origin);
						float angle = (entity_vector - origin).atan2();
						living_entity->hitByExplosion(damage_scale, radius, distance, angle);
						living_entity->increaseUpdateCounter();
						living_entity->sendToVisible();
						if (living_entity->isPlayer()) {
							ServerPlayerPtr player = std::dynamic_pointer_cast<ServerPlayer>(living_entity);
							living_entity->sendTo(*player->getClient());
						}
					}
				}
			}

			if (options.affectsTileEntities) {
				if (TileEntityPtr tile = realm.tileEntityAt(position); tile && tile->kill()) {
					tile->destroy();
				}
			}
		});
	}

	void causeExplosion(const Place &place, float radius, float damage_scale, bool affects_tile_entities) {
		ExplosionOptions options{
			.particleType = "base:entity/explosion_particle",
			.radius = radius,
			.damageScale = damage_scale,
			.affectsTileEntities = affects_tile_entities,
		};

		causeExplosion(place, options);
	}
}
