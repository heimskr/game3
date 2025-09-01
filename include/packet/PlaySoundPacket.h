#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Position.h"

namespace Game3 {
	class Entity;

	struct PlaySoundPacket: Packet {
		static PacketID ID() { return 63; }

		Identifier soundID;
		Position soundOrigin;
		float pitch = 1;
		uint16_t maximumDistance = 65535;

		PlaySoundPacket() = default;
		PlaySoundPacket(Identifier soundID, Position soundOrigin, float pitch = 1, uint16_t maximumDistance = 65535):
			soundID(std::move(soundID)), soundOrigin(soundOrigin), pitch(pitch), maximumDistance(maximumDistance) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << soundID << soundOrigin << pitch << maximumDistance; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> soundID >> soundOrigin >> pitch >> maximumDistance; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
