#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class LivingEntity;
	class Village;

	struct VillageUpdatePacket: Packet {
		static PacketID ID() { return 57; }

		VillageID villageID{};
		RealmID realmID{};
		ChunkPosition chunkPosition;
		Position position;
		std::string name;
		LaborAmount labor{};
		double greed{};
		Resources resources{};

		VillageUpdatePacket() = default;
		explicit VillageUpdatePacket(const Village &);
		VillageUpdatePacket(VillageID village_id, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, std::string name_, LaborAmount labor_, double greed_, Resources resources_):
			villageID(village_id), realmID(realm_id), chunkPosition(chunk_position), position(position_), name(std::move(name_)), labor(labor_), greed(greed_), resources(std::move(resources_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << villageID << realmID << chunkPosition << position << name << labor << greed << resources; }
		void decode(Game &, Buffer &buffer)       override { buffer >> villageID >> realmID >> chunkPosition >> position >> name >> labor >> greed >> resources; }

		void handle(ClientGame &) override;
	};
}
