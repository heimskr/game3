#pragma once

#include "net/SendBuffer.h"
#include "packet/Packet.h"
#include "threading/Lockable.h"

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <string>

#include <asio.hpp>
#include <asio/ssl.hpp>

namespace Game3 {
	class Realm;
	class Server;
	class ServerPlayer;
	struct ChunkPosition;

	class BufferGuard {
		public:
			virtual ~BufferGuard() = default;
	};

	class GenericClient: public std::enable_shared_from_this<GenericClient> {
		public:
			std::weak_ptr<Server> weakServer;
			int id = -1;
			std::string ip;
			SendBuffer sendBuffer;

			GenericClient(const GenericClient &) = delete;
			GenericClient(GenericClient &&) = delete;
			GenericClient(const std::shared_ptr<Server> &server, std::string_view ip, int id);

			virtual ~GenericClient();

			GenericClient & operator=(const GenericClient &) = delete;
			GenericClient & operator=(GenericClient &&) = delete;

			virtual void start() = 0;
			virtual bool send(const PacketPtr &) = 0;
			virtual void handleInput(std::string_view) = 0;
			virtual bool isBuffering() const = 0;
			virtual void close() = 0;
			virtual void removeSelf() = 0;
			virtual void send(std::string, bool force) = 0;
			virtual std::unique_ptr<BufferGuard> bufferGuard() = 0;

			void sendChunk(Realm &, ChunkPosition, bool can_request = true, uint64_t counter_threshold = 0);

			inline auto getPlayer() const { return weakPlayer.lock(); }
			inline void setPlayer(const std::shared_ptr<ServerPlayer> &shared) { weakPlayer = shared; }

			inline void setClosed() { closed = true; }
			inline bool isClosed() const { return closed; }

			inline std::shared_ptr<Server> getServer() { return weakServer.lock(); }

		protected:
			std::weak_ptr<ServerPlayer> weakPlayer;
			bool closed = false;

			GenericClient(const std::shared_ptr<Server> &);
	};

	using GenericClientPtr = std::shared_ptr<GenericClient>;
}
