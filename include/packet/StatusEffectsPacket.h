#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "statuseffect/StatusEffectMap.h"

namespace Game3 {
	class Entity;

	struct StatusEffectsPacket: Packet {
		static PacketID ID() { return 70; }

		GlobalID globalID = -1;
		StatusEffectMap map;

		StatusEffectsPacket() = default;
		StatusEffectsPacket(LivingEntity &);
		StatusEffectsPacket(GlobalID globalID, StatusEffectMap map);

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &buffer) const final { buffer << globalID << map; }
		void decode(Game &, BasicBuffer &buffer)       final { buffer >> globalID >> map; }

		void handle(const std::shared_ptr<ClientGame> &) final;
	};
}
