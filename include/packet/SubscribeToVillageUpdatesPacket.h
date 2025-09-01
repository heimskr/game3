#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct SubscribeToVillageUpdatesPacket: Packet {
		static PacketID ID() { return 56; }

		std::optional<VillageID> villageID;

		SubscribeToVillageUpdatesPacket() = default;
		SubscribeToVillageUpdatesPacket(VillageID village_id):
			villageID(village_id) {}
		SubscribeToVillageUpdatesPacket(const std::optional<VillageID> &village_id):
			villageID(village_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << villageID; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> villageID; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
