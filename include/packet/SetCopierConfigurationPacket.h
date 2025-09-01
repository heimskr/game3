#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct SetCopierConfigurationPacket: Packet {
		static PacketID ID() { return 62; }

		Slot slot = -1;
		bool includeTileEntities{};

		SetCopierConfigurationPacket() = default;
		SetCopierConfigurationPacket(Slot slot_, bool include_tile_entities):
			slot(slot_), includeTileEntities(include_tile_entities) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << slot << includeTileEntities; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> slot >> includeTileEntities; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
