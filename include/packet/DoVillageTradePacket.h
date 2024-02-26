#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct DoVillageTradePacket: Packet {
		static PacketID ID() { return 59; }

		VillageID villageID;
		Identifier resource;
		ItemCount amount{};
		bool isSell{};

		DoVillageTradePacket() = default;
		DoVillageTradePacket(VillageID village_id, Identifier resource_, ItemCount amount_, bool is_sell):
			villageID(village_id), resource(std::move(resource_)), amount(amount_), isSell(is_sell) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << villageID << resource << amount << isSell; }
		void decode(Game &, Buffer &buffer)       override { buffer >> villageID >> resource >> amount >> isSell; }

		void handle(const std::shared_ptr<ServerGame> &, RemoteClient &) override;
	};
}
