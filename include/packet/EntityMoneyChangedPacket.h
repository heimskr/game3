#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Direction.h"
#include "types/Position.h"

namespace Game3 {
	class Entity;

	struct EntityMoneyChangedPacket: Packet {
		static PacketID ID() { return 60; }

		GlobalID globalID{};
		MoneyCount moneyCount{};

		EntityMoneyChangedPacket() = default;
		EntityMoneyChangedPacket(const Entity &);
		EntityMoneyChangedPacket(GlobalID global_id, MoneyCount money_count):
			globalID(global_id), moneyCount(money_count) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << moneyCount; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> moneyCount; }

		void handle(ClientGame &) override;
	};
}
