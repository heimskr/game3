#include "entity/EntityFactory.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
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
		randomizationParameters(std::move(options.randomizationParameters)),
		realmID(realmID),
		radius(options.radius),
		particleCount(options.particleCount) {}

	void ExplosionPacket::encode(Game &, Buffer &buffer) const {
		buffer << realmID << origin << radius << particleCount << particleType << randomizationParameters;
	}

	void ExplosionPacket::decode(Game &game, Buffer &buffer) {
		randomizationParameters.context = game.shared_from_this();
		buffer >> realmID >> origin >> radius >> particleCount >> particleType >> randomizationParameters;
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

		for (uint32_t i = 0; i < particleCount; ++i) {
			EntityPtr entity = (*factory)(game);
			entity->setRandomizationParameters(randomizationParameters);
			entity->velocity = randomOnSphereUpperHalf(threadContext.rng) * radius;
			realm->spawn(entity, origin);
		}
	}
}
