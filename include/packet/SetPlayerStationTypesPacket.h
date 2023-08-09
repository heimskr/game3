#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct SetPlayerStationTypesPacket: Packet {
		static PacketID ID() { return 45; }

		std::unordered_set<Identifier> stationTypes;

		SetPlayerStationTypesPacket() = default;
		SetPlayerStationTypesPacket(std::unordered_set<Identifier> station_types):
			stationTypes(std::move(station_types)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << stationTypes; }
		void decode(Game &, Buffer &buffer)       override { buffer >> stationTypes; }

		void handle(ClientGame &) override;
	};
}
