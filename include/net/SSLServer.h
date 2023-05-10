#pragma once

#include <functional>
#include <map>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <set>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

#include "net/Server.h"

namespace Game3 {
	class SSLServer: public Server {
		public:
			SSLServer(int af_, const std::string &ip_, uint16_t port_, const std::string &cert, const std::string &key, size_t thread_count, size_t chunk_size = 1);
			SSLServer(const SSLServer &) = delete;
			SSLServer(SSLServer &&) = delete;

			~SSLServer() override;

			SSLServer & operator=(const SSLServer &) = delete;
			SSLServer & operator=(SSLServer &&) = delete;

		public:
			SSL_CTX *sslContext = nullptr;
			std::mutex sslContextMutex;

			/** Maps descriptors to SSL pointers. */
			std::map<int, SSL *> ssls;
			std::mutex sslsMutex;

			/** Maps descriptors to mutexes that need to be locked when using an SSL object. */
			std::map<int, std::mutex> sslMutexes;

			std::shared_ptr<Worker> makeWorker(size_t buffer_size, size_t id) override;

			struct Worker: Server::Worker {
				using Server::Worker::Worker;

				void remove(bufferevent *) override;
				void accept(int new_fd) override;
			};

			friend struct Worker;
	};
}
