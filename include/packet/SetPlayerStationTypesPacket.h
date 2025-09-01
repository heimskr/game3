#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct SetPlayerStationTypesPacket: Packet {
		static PacketID ID() { return 45; }

		std::unordered_set<Identifier> stationTypes;
		bool focus = false;

		SetPlayerStationTypesPacket() = default;
		SetPlayerStationTypesPacket(std::unordered_set<Identifier> station_types, bool focus_):
			stationTypes(std::move(station_types)), focus(focus_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << stationTypes << focus; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> stationTypes >> focus; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
