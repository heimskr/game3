#pragma once

#include <optional>

#include "game/Game.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct ContinuousInteractionPacket: Packet {
		static PacketID ID() { return 32; }

		std::optional<uint8_t> modifiers;

		ContinuousInteractionPacket() = default;
		ContinuousInteractionPacket(Modifiers modifiers_):
			modifiers(static_cast<uint8_t>(modifiers_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << modifiers; }
		void decode(Game &, Buffer &buffer)       override { buffer >> modifiers; }

		void handle(const std::shared_ptr<ServerGame> &, RemoteClient &) override;
	};
}
