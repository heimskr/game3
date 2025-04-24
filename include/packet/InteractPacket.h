#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct InteractPacket: Packet {
		static PacketID ID() { return 21; }

		bool direct = false;
		Hand hand = Hand::None;
		Modifiers modifiers;
		std::optional<GlobalID> globalID;
		std::optional<Direction> direction;

		InteractPacket() = default;
		InteractPacket(bool direct, Hand hand, Modifiers modifiers, std::optional<GlobalID> globalID = {}, std::optional<Direction> direction = {}):
			direct(direct), hand(hand), modifiers(modifiers), globalID(globalID), direction(direction) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << direct << hand << modifiers << globalID << direction; }
		void decode(Game &, Buffer &buffer)       override { buffer >> direct >> hand >> modifiers >> globalID >> direction; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
