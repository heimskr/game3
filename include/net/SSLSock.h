#pragma once

#include <cerrno>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <resolv.h>
#include <cstring>
#include <unistd.h>

#include "net/Sock.h"

namespace Game3 {
	class SSLSock: public Sock {
		public:
			using Sock::Sock;

			void connect() override;
			ssize_t send(const void *, size_t) override;
			ssize_t recv(void *, size_t) override;

		protected:
			SSL_CTX *sslContext = nullptr;
			SSL *ssl = nullptr;

			void connectSSL();
	};
}
