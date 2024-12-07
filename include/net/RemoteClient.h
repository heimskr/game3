#pragma once

#include "net/Buffer.h"
#include "net/GenericClient.h"
#include "packet/Packet.h"

#include <format>
#include <memory>

namespace Game3 {
	class RemoteClient: public GenericClient {
		public:
			struct RemoteBufferGuard: public BufferGuard {
				std::weak_ptr<RemoteClient> weakParent;

				RemoteBufferGuard(RemoteClient &parent):
					RemoteBufferGuard(std::static_pointer_cast<RemoteClient>(parent.shared_from_this())) {}

				RemoteBufferGuard(const std::shared_ptr<RemoteClient> &parent):
					weakParent(parent) {
						parent->startBuffering();
					}

				RemoteBufferGuard(const RemoteBufferGuard &) = delete;
				RemoteBufferGuard(RemoteBufferGuard &&) noexcept = default;

				~RemoteBufferGuard() override {
					if (auto locked = weakParent.lock())
						locked->stopBuffering();
				}

				RemoteBufferGuard & operator=(const RemoteBufferGuard &) = delete;
				RemoteBufferGuard & operator=(RemoteBufferGuard &&) noexcept = default;
			};

			constexpr static size_t MAX_PACKET_SIZE = 1 << 24;

			asio::ssl::stream<asio::ip::tcp::socket> socket;
			asio::io_context::strand strand;
			Lockable<std::deque<std::string>, std::shared_mutex> outbox;

			RemoteClient(const std::shared_ptr<Server> &, std::string_view ip, int id_, asio::ip::tcp::socket &&socket);

			~RemoteClient() override;

			void queue(std::string);

			void start() override;
			void handleInput(std::string_view) override;
			bool send(const PacketPtr &) override;
			void send(std::string, bool force) override;

			std::unique_ptr<BufferGuard> bufferGuard() final { return std::make_unique<RemoteBufferGuard>(*this); }

			void startBuffering();
			void flushBuffer(bool force = false);
			void stopBuffering();
			bool isBuffering() const override;

			void close() final;
			void removeSelf() override;

		protected:
			using GenericClient::GenericClient;

		private:
			size_t bufferSize;
			std::unique_ptr<char[]> buffer;
			enum class State {Begin, Data};
			State state = State::Begin;
			std::vector<uint8_t> headerBytes;
			uint16_t packetType = 0;
			uint32_t payloadSize = 0;
			Buffer receiveBuffer{Side::Client};

			inline auto getSelf() { return std::static_pointer_cast<RemoteClient>(shared_from_this()); }
			inline auto getSelf() const { return std::static_pointer_cast<const RemoteClient>(shared_from_this()); }

			void mock();
			void write();
			void writeHandler(const asio::error_code &, size_t);
			void doHandshake();
			void doRead();
	};

	using RemoteClientPtr = std::shared_ptr<RemoteClient>;
}
