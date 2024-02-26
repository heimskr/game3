#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class TileEntity;

	class TileEntityPacket: public Packet {
		public:
			static PacketID ID() { return 2; }

			std::shared_ptr<TileEntity> tileEntity;
			Identifier identifier;
			GlobalID globalID = -1;
			RealmID realmID = -1;

			TileEntityPacket() = default;
			TileEntityPacket(std::shared_ptr<TileEntity>);

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &) const override;
			void decode(Game &, Buffer &) override;

			void handle(const std::shared_ptr<ClientGame> &) override;

		private:
			Buffer storedBuffer;
			bool wasFound = false;
	};
}
