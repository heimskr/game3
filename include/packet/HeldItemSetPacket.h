#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct HeldItemSetPacket: Packet {
		static PacketID ID() { return 34; }

		RealmID realmID = -1;
		GlobalID entityID = -1;
		bool leftHand = false;
		Slot slot = -1;

		HeldItemSetPacket() = default;
		HeldItemSetPacket(RealmID realm_id, GlobalID entity_id, bool left_hand, Slot slot_):
			realmID(realm_id), entityID(entity_id), leftHand(left_hand), slot(slot_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << entityID << leftHand << slot; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> entityID >> leftHand >> slot; }

		void handle(ClientGame &) override;
	};
}
