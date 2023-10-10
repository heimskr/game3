#pragma once

#include <span>
#include <string>

#include <openssl/evp.h>
#include <openssl/sha.h>

namespace Game3 {
	template <typename T>
	T computeSHA3_512(std::string_view input);

	/** Returns a secret hex string of a variable length. The length will be in the range [count, 16 * count] (not an even distribution). */
	std::string generateSecret(size_t count);

	class Hasher {
		public:
			enum class Algorithm {SHA3_256, SHA3_512};
			Hasher(Algorithm);
			~Hasher();

			Hasher & operator+=(std::string_view);

			template <typename T>
			Hasher & operator+=(std::span<T> span) {
				return *this += std::string_view(reinterpret_cast<const char *>(span.data()), span.size_bytes());
			}

			template <typename T>
			T value();

			operator bool() const;

		private:
			Algorithm algorithm;
			uint32_t digestLength{};
			const EVP_MD *evpAlgorithm = nullptr;
			uint8_t *digest = nullptr;
			EVP_MD_CTX *context = nullptr;
			bool alive = true;

			void checkAlive() const;
	};
}
