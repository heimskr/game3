#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Direction.h"
#include "types/Position.h"

namespace Game3 {
	struct OpenVillageTradePacket: Packet {
		static PacketID ID() { return 58; }

		VillageID villageID{};
		bool removeOnMove = true;

		OpenVillageTradePacket() = default;
		OpenVillageTradePacket(VillageID village_id, bool remove_on_move = true):
			villageID(village_id), removeOnMove(remove_on_move) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << villageID << removeOnMove; }
		void decode(Game &, Buffer &buffer)       override { buffer >> villageID >> removeOnMove; }

		void handle(ClientGame &) override;
	};
}
