#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "statuseffect/StatusEffectMap.h"

namespace Game3 {
	class Entity;
	struct ExplosionOptions;

	struct ExplosionPacket: Packet {
		static PacketID ID() { return 72; }

		Position origin;
		std::string type = "default";
		Identifier particleType = "base:entity/square_particle";
		Buffer randomizationParameters{Side::Client};
		RealmID realmID{};
		float radius{};
		uint32_t particleCount{};

		ExplosionPacket() = default;
		ExplosionPacket(RealmID realmID, Position origin, ExplosionOptions);

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &) const override;
		void decode(Game &, Buffer &) override;

		void handle(const std::shared_ptr<ClientGame> &) final;
	};
}
