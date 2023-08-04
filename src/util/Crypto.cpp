#include <cassert>
#include <memory>
#include <random>
#include <sstream>
#include <vector>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include "util/Crypto.h"
#include "util/Math.h"

namespace Game3 {
	template <>
	std::string computeSHA3(std::string_view input) {
		uint32_t digest_length = SHA512_DIGEST_LENGTH;
		const auto *algorithm = EVP_sha3_512();
		auto *digest = static_cast<uint8_t *>(OPENSSL_malloc(digest_length));
		auto *context = EVP_MD_CTX_new();
		EVP_DigestInit_ex(context, algorithm, nullptr);
		EVP_DigestUpdate(context, input.data(), input.size());
		EVP_DigestFinal_ex(context, digest, &digest_length);
		EVP_MD_CTX_destroy(context);
		std::string out(reinterpret_cast<const char *>(digest), static_cast<size_t>(digest_length));
		OPENSSL_free(digest);
		return out;
	}

	template <>
	std::vector<uint8_t> computeSHA3(std::string_view input) {
		uint32_t digest_length = SHA512_DIGEST_LENGTH;
		const auto *algorithm = EVP_sha3_512();
		auto *digest = static_cast<uint8_t *>(OPENSSL_malloc(digest_length));
		auto *context = EVP_MD_CTX_new();
		EVP_DigestInit_ex(context, algorithm, nullptr);
		EVP_DigestUpdate(context, input.data(), input.size());
		EVP_DigestFinal_ex(context, digest, &digest_length);
		EVP_MD_CTX_destroy(context);
		std::vector out(digest, digest + digest_length);
		OPENSSL_free(digest);
		return out;
	}

	template <>
	uint64_t computeSHA3(std::string_view input) {
		const auto bytes = computeSHA3<std::vector<uint8_t>>(input);
		assert(sizeof(uint64_t) <= bytes.size());
		return toLittle(*reinterpret_cast<const uint64_t *>(bytes.data()));
	}

	std::string generateSecret(size_t count) {
		std::stringstream ss;
		std::random_device rng;
		std::mt19937_64 prng(rng());
		for (size_t i = 0; i < count; ++i)
			ss << std::hex << prng();
		return ss.str();
	}
}
