#pragma once

#include <cassert>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>

#include "data/UserInfo.h"
#include "net/ApplicationServer.h"
#include "net/GenericClient.h"
#include "net/Server.h"
#include "util/Math.h"

namespace Game3 {
	class RemoteClient;
	class ServerGame;
	class ServerPlayer;

	/** Used by servers to represent themselves. */
	class LocalServer: public ApplicationServer {
		public:
			std::shared_ptr<Server> server;
			std::shared_ptr<ServerGame> game;

			LocalServer() = delete;
			LocalServer(const LocalServer &) = delete;
			LocalServer(LocalServer &&) = delete;
			LocalServer(std::shared_ptr<Server>, std::string_view secret_);

			~LocalServer() override;

			LocalServer & operator=(const LocalServer &) = delete;
			LocalServer & operator=(LocalServer &&) = delete;

			void run() override;
			void stop() override;
			void send(GenericClient &, std::string_view);
			void readUsers();
			/** Saves username and display name information. */
			void saveUsers();
			/** Writes every player's full data to the database. */
			void saveUserData();
			/** Returns the display name if authentication was successful. */
			std::optional<std::string> authenticate(const std::string &username, Token) const;
			std::shared_ptr<ServerPlayer> loadPlayer(std::string_view username, std::string_view display_name);
			bool hasUsername(const std::string &) const;
			bool hasDisplayName(const std::string &) const;
			Token generateToken(const std::string &username) const;
			void setupPlayer(RemoteClient &);

			static bool validateUsername(std::string_view);
			static int main(int argc, char **argv);

			template <std::integral T>
			void send(GenericClient &client, T value) {
				assert(server);
				const T little = toLittle(value);
				server->send(client, std::string_view(reinterpret_cast<const char *>(&little), sizeof(T)));
			}

		private:
			std::string secret;
			std::map<std::string, UserInfo> userDatabase;
			std::optional<std::filesystem::path> userDatabasePath;
			std::set<std::string> displayNames;
	};

}
