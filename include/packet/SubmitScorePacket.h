#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct SubmitScorePacket: Packet {
		static PacketID ID() { return 66; }

		Identifier minigameID;
		uint64_t score{};

		SubmitScorePacket() = default;
		SubmitScorePacket(Identifier minigame_id, uint64_t score);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << minigameID << score; }
		void decode(Game &, Buffer &buffer)       override { buffer >> minigameID >> score; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
