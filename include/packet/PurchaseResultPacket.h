#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "statuseffect/StatusEffectMap.h"

namespace Game3 {
	class Entity;

	struct PurchaseResultPacket: Packet {
		static PacketID ID() { return 74; }

		bool success{};

		PurchaseResultPacket() = default;
		PurchaseResultPacket(bool success):
			success(success) {}

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &buffer) const final { buffer << success; }
		void decode(Game &, Buffer &buffer)       final { buffer >> success; }

		void handle(const std::shared_ptr<ClientGame> &) final;
	};
}
