#pragma once

#include <atomic>
#include <netdb.h>
#include <shared_mutex>
#include <string>
#include <vector>

#ifdef __MINGW64__
#include <winsock.h>
#endif

namespace Game3 {
	class Sock {
		friend class SocketBuffer;

		public:
			const std::string hostname;
			uint16_t port;

			Sock(std::string_view hostname_, uint16_t port_);

			Sock() = delete;
			Sock(const Sock &) = delete;
			Sock & operator=(const Sock &) = delete;

			virtual ~Sock();

			/** Connects to the socket. */
			virtual void connect(bool blocking);

			/** Closes the socket. */
			virtual void close(bool force);

			/** Sends a given number of bytes from a buffer through the socket and returns the number of bytes sent. */
			virtual ssize_t send(const void *, size_t, bool force);

			/** Reads a given number of bytes into a buffer from the socket and returns the number of bytes read. */
			virtual ssize_t recv(void *, size_t);

			void startBuffering();
			void flushBuffer(bool do_lock = true);
			void stopBuffering();

			inline bool isConnected() const { return connected; }
			virtual bool isReady() const { return connected; }

		protected:
			static int sockCount;
			struct addrinfo *info;
			int netFD = -1, controlRead = -1, controlWrite = -1;
			std::atomic_bool connected = false;
			fd_set fds = {0};
			std::vector<char> buffer;
			std::atomic_bool buffering = false;
			std::shared_mutex bufferMutex;

			enum class ControlMessage: char {Close='C'};

			void addToBuffer(const void *, size_t);
	};
}
