#pragma once

#include "types/ChunkPosition.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class TimePacket: public Packet {
		public:
			static PacketID ID() { return 30; }

			double time;

			TimePacket() = default;
			TimePacket(double time_): time(time_) {}

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &buffer) const override { buffer << time; }
			void decode(Game &, BasicBuffer &buffer)       override { buffer >> time; }

			void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
