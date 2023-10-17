#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "pipes/ItemFilter.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct SetItemFiltersPacket: Packet {
		static PacketID ID() { return 50; }

		GlobalID pipeGID;
		Direction direction;
		ItemFilter itemFilter;

		SetItemFiltersPacket() = default;
		SetItemFiltersPacket(GlobalID pipe_gid, Direction direction_, ItemFilter item_filter):
			pipeGID(pipe_gid), direction(direction_), itemFilter(std::move(item_filter)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << pipeGID << direction << itemFilter; }
		void decode(Game &, Buffer &buffer)       override { buffer >> pipeGID >> direction >> itemFilter; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
