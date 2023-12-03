#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class LivingEntity;

	struct LivingEntityHealthChangedPacket: Packet {
		static PacketID ID() { return 54; }

		GlobalID globalID = -1;
		HitPoints newHealth;

		LivingEntityHealthChangedPacket() = default;
		explicit LivingEntityHealthChangedPacket(const LivingEntity &);
		LivingEntityHealthChangedPacket(GlobalID global_id, HitPoints new_health):
			globalID(global_id), newHealth(new_health) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << newHealth; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> newHealth; }

		void handle(ClientGame &) override;
	};
}
