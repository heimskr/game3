#pragma once

#include "net/GenericClient.h"
#include "packet/Packet.h"

namespace Game3 {
	class GameServer;

	class GameClient: public GenericClient {
		public:
			GameServer &server;

			GameClient() = delete;
			GameClient(GameServer &server_, int id_, std::string_view ip_):
				GenericClient(id_, ip_), server(server_) {}

			~GameClient() override = default;

			void handleInput(std::string_view) override;
			void send(Packet &);

			template <typename T>
			requires (!std::derived_from<T, Packet>)
			void send(const T &value);
	};
}
