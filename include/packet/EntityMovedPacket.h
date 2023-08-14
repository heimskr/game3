#pragma once

#include <optional>

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct EntityMovedPacket: Packet {
		struct Args {
			GlobalID globalID = -1;
			RealmID realmID = -1;
			Position position;
			Direction facing = Direction::Invalid;
			std::optional<Offset> offset;
			std::optional<float> zSpeed;
			bool adjustOffset = false;
			bool isTeleport = false;
		};

		static PacketID ID() { return 17; }

		Args arguments;

		EntityMovedPacket() = default;
		EntityMovedPacket(const Entity &);
		EntityMovedPacket(const Args &arguments_): arguments(arguments_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &) const override;
		void decode(Game &, Buffer &) override;

		void handle(ClientGame &) override;
	};
}
