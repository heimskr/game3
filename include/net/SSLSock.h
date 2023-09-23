#pragma once

#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <optional>
#include <resolv.h>
#include <thread>
#include <unistd.h>

#include "net/Sock.h"

namespace Game3 {
	class SSLSock: public Sock {
		public:
			using Sock::Sock;

			void connect(bool blocking) override;
			void close(bool force) override;
			ssize_t send(const void *, size_t, bool force) override;
			ssize_t recv(void *, size_t) override;
			bool isReady() const override;

		protected:
			SSL_CTX *sslContext = nullptr;
			SSL *ssl = nullptr;

			void connectSSL(bool blocking);
			std::optional<std::thread::id> threadID;
	};
}
