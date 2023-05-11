#pragma once

#include "Types.h"

namespace Game3 {
	class Buffer;
	class Game;

	class Packet {
		public:
			Packet() = default;
			virtual ~Packet() = default;

			virtual void encode(Game &, Buffer &) const = 0;
			virtual void decode(Game &, Buffer &) = 0;
			virtual PacketID getID() const = 0;
	};
}
