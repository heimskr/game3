#include "packet/Packet.h"

namespace Game3 {
	class TileEntity;

	struct TileEntityPacket: Packet {
		TileEntityPtr tileEntity;
		Identifier identifier;
		GlobalID globalID = -1;
		RealmID realmID = -1;

		TileEntityPacket() = default;
		TileEntityPacket(std::shared_ptr<TileEntity>);

		void encode(Game &, Buffer &) override;
		void decode(Game &, Buffer &) override;
	};
}
