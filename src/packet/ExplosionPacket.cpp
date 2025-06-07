#include "entity/EntityFactory.h"
#include "entity/ExplosionParticle.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "math/FilledCircle.h"
#include "math/Random.h"
#include "net/GenericClient.h"
#include "packet/ExplosionPacket.h"
#include "threading/ThreadContext.h"
#include "util/Explosion.h"

namespace Game3 {
	ExplosionPacket::ExplosionPacket(RealmID realmID, Position origin, ExplosionOptions options):
		origin(origin),
		type(std::move(options.type)),
		particleType(std::move(options.particleType)),
		soundEffect(std::move(options.soundEffect)),
		randomizationParameters(std::move(options.randomizationParameters)),
		pitchVariance(options.pitchVariance),
		realmID(realmID),
		radius(options.radius) {}

	void ExplosionPacket::encode(Game &, Buffer &buffer) const {
		buffer << realmID << origin << radius << particleType << soundEffect << pitchVariance << randomizationParameters;
	}

	void ExplosionPacket::decode(Game &game, Buffer &buffer) {
		randomizationParameters.context = game.shared_from_this();
		buffer >> realmID >> origin >> radius >> particleType >> soundEffect >> pitchVariance >> randomizationParameters;
	}

	void ExplosionPacket::handle(const std::shared_ptr<ClientGame> &game) {
		RealmPtr realm = game->tryRealm(realmID);
		if (!realm) {
			return;
		}

		auto factory = game->registry<EntityFactoryRegistry>().maybe(particleType);
		if (!factory) {
			return;
		}

		if (soundEffect) {
			realm->playSound(origin, *soundEffect, threadContext.getPitch(pitchVariance.value_or(1.f)), std::max<uint16_t>(radius * 2, 64));
		}

		std::optional<float> lifetime;
		if (particleType == ExplosionParticle::ID()) {
			lifetime = ExplosionParticle::getLifetime();
		}

		iterateFilledCircle<Position::IntType>(origin.column, origin.row, radius, [&](auto x, auto y) {
			EntityPtr entity = (*factory)(game);
			entity->setRandomizationParameters(randomizationParameters);

			Position position{y, x};
			if (lifetime) {
				float distance = position.distance(origin);
				entity->age = -distance / radius * *lifetime / 3.14159265358979323846;
			}

			realm->spawn(entity, position);
		});
	}
}
