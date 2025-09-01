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
		UpdateCounter newCounter = 0;

		HeldItemSetPacket() = default;
		HeldItemSetPacket(RealmID realm_id, GlobalID entity_id, bool left_hand, Slot slot_, UpdateCounter new_counter):
			realmID(realm_id), entityID(entity_id), leftHand(left_hand), slot(slot_), newCounter(new_counter) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << entityID << leftHand << slot << newCounter; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> realmID >> entityID >> leftHand >> slot >> newCounter; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
