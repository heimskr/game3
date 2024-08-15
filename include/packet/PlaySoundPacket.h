#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Direction.h"
#include "types/Position.h"

namespace Game3 {
	class Entity;

	struct PlaySoundPacket: Packet {
		static PacketID ID() { return 63; }

		Identifier soundID;
		Position soundOrigin;

		PlaySoundPacket() = default;
		PlaySoundPacket(Identifier sound_id, const Position &sound_origin):
			soundID(std::move(sound_id)), soundOrigin(sound_origin) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << soundID << soundOrigin; }
		void decode(Game &, Buffer &buffer)       override { buffer >> soundID >> soundOrigin; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
