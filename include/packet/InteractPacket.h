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
		InteractPacket(bool direct_, Hand hand_, Modifiers modifiers_, std::optional<GlobalID> global_id = {}, std::optional<Direction> direction_ = {}):
			direct(direct_), hand(hand_), modifiers(modifiers_), globalID(global_id), direction(direction_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << direct << hand << modifiers << globalID << direction; }
		void decode(Game &, Buffer &buffer)       override { buffer >> direct >> hand >> modifiers >> globalID >> direction; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
