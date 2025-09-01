#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct OpenMinigamePacket: Packet {
		static PacketID ID() { return 67; }

		Identifier minigameID;
		int gameWidth{};
		int gameHeight{};

		OpenMinigamePacket() = default;
		OpenMinigamePacket(Identifier minigame_id, int game_width, int game_height);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << minigameID << gameWidth << gameHeight; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> minigameID >> gameWidth >> gameHeight; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
