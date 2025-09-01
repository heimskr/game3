#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct DestroyEntityPacket: Packet {
		static PacketID ID() { return 23; }

		GlobalID globalID = -1;
		std::optional<RealmID> realmRequirement;

		DestroyEntityPacket() = default;

		DestroyEntityPacket(const Entity &, bool require_realm);

		explicit DestroyEntityPacket(GlobalID global_id):
			globalID(global_id) {}

		DestroyEntityPacket(GlobalID global_id, RealmID realm_requirement):
			globalID(global_id), realmRequirement(realm_requirement) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmRequirement; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> globalID >> realmRequirement; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
