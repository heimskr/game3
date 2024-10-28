#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct CraftPacket: Packet {
		static PacketID ID() { return 31; }

		GlobalID packetID = -1;
		uint64_t recipeIndex = -1;
		uint64_t count = -1;

		CraftPacket() = default;
		CraftPacket(GlobalID packet_id, uint64_t recipe_index, uint64_t count_):
			packetID(packet_id), recipeIndex(recipe_index), count(count_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << packetID << recipeIndex << count; }
		void decode(Game &, Buffer &buffer)       override { buffer >> packetID >> recipeIndex >> count; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
