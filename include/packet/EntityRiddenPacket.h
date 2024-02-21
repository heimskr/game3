#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Direction.h"
#include "types/Position.h"

namespace Game3 {
	class Entity;

	struct EntityRiddenPacket: Packet {
		static PacketID ID() { return 61; }

		std::optional<GlobalID> riderID;
		GlobalID riddenID{};

		EntityRiddenPacket() = default;
		EntityRiddenPacket(const std::shared_ptr<Entity> &rider, const Entity &ridden);
		EntityRiddenPacket(std::optional<GlobalID> rider_id, GlobalID ridden_id):
			riderID(rider_id), riddenID(ridden_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << riderID << riddenID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> riderID >> riddenID; }

		void handle(ClientGame &) override;
	};
}
