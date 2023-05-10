#pragma once

#include <cassert>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "net/ApplicationServer.h"
#include "net/GenericClient.h"
#include "net/Server.h"
#include "util/Math.h"

namespace Game3 {
	class Game;

	class GameServer: public ApplicationServer {
		public:
			std::shared_ptr<Server> server;
			std::shared_ptr<Game> game;

			GameServer() = delete;
			GameServer(const GameServer &) = delete;
			GameServer(GameServer &&) = delete;
			GameServer(std::shared_ptr<Server>);

			~GameServer() override;

			GameServer & operator=(const GameServer &) = delete;
			GameServer & operator=(GameServer &&) = delete;

			void run() override;
			void stop() override;
			void send(const GenericClient &, std::string_view);
			void send(int, std::string_view);

			static int main(int argc, char **argv);

			template <std::integral T>
			void send(const GenericClient &client, T value) {
				send(client.id, value);
			}

			template <std::integral T>
			void send(int id, T value) {
				assert(server);
				const T little = toLittle(value);
				server->send(id, std::string_view(reinterpret_cast<const char *>(&little), sizeof(T)));
			}
	};

}
