#include "Log.h"
#include "util/Crypto.h"
#include "util/Math.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <random>
#include <sstream>
#include <vector>

namespace Game3 {
	template <>
	std::string computeSHA3_512(std::string_view input) {
		uint32_t digest_length = SHA512_DIGEST_LENGTH;
		const auto *algorithm = EVP_sha3_512();
		auto *digest = static_cast<uint8_t *>(OPENSSL_malloc(digest_length));
		auto *context = EVP_MD_CTX_new();
		EVP_DigestInit_ex(context, algorithm, nullptr);
		EVP_DigestUpdate(context, input.data(), input.size());
		EVP_DigestFinal_ex(context, digest, &digest_length);
		EVP_MD_CTX_destroy(context);
		try {
			std::string out(reinterpret_cast<const char *>(digest), static_cast<size_t>(digest_length));
			OPENSSL_free(digest);
			return out;
		} catch (...) {
			OPENSSL_free(digest);
			throw;
		}
	}

	template <>
	std::vector<uint8_t> computeSHA3_512(std::string_view input) {
		uint32_t digest_length = SHA512_DIGEST_LENGTH;
		const auto *algorithm = EVP_sha3_512();
		auto *digest = static_cast<uint8_t *>(OPENSSL_malloc(digest_length));
		auto *context = EVP_MD_CTX_new();
		EVP_DigestInit_ex(context, algorithm, nullptr);
		EVP_DigestUpdate(context, input.data(), input.size());
		EVP_DigestFinal_ex(context, digest, &digest_length);
		EVP_MD_CTX_destroy(context);
		try {
			std::vector out(digest, digest + digest_length);
			OPENSSL_free(digest);
			return out;
		} catch (...) {
			OPENSSL_free(digest);
			throw;
		}
	}

	template <>
	uint64_t computeSHA3_512(std::string_view input) {
		const auto bytes = computeSHA3_512<std::vector<uint8_t>>(input);
		assert(sizeof(uint64_t) <= bytes.size());
		uint64_t out{};
		std::memcpy(&out, bytes.data(), sizeof(uint64_t));;
		return toLittle(out);
	}

	Hasher::Hasher(Algorithm algorithm_): algorithm(algorithm_) {
		switch (algorithm) {
			case Algorithm::SHA3_256:
				evpAlgorithm = EVP_sha3_256();
				digestLength = SHA256_DIGEST_LENGTH;
				break;

			case Algorithm::SHA3_512:
				evpAlgorithm = EVP_sha3_512();
				digestLength = SHA512_DIGEST_LENGTH;
				break;

			default:
				throw std::runtime_error("Invalid algorithm: " + std::to_string(int(algorithm)));
		}

		context = EVP_MD_CTX_new();
		digest = static_cast<uint8_t *>(OPENSSL_malloc(digestLength));
		EVP_DigestInit_ex(context, evpAlgorithm, nullptr);
	}

	Hasher::~Hasher() {
		if (alive) {
			alive = false;
			EVP_MD_CTX_destroy(context);
			OPENSSL_free(digest);
		}
	}

	void Hasher::checkAlive() const {
		if (!alive)
			throw std::runtime_error("Hasher isn't alive");
	}

	Hasher & Hasher::operator+=(std::string_view input) {
		checkAlive();
		EVP_DigestUpdate(context, input.data(), input.size());
		return *this;
	}

	template<>
	std::string Hasher::value() {
		EVP_DigestFinal_ex(context, digest, &digestLength);
		EVP_MD_CTX_destroy(context);
		alive = false;
		try {
			std::string out(reinterpret_cast<const char *>(digest), static_cast<size_t>(digestLength));
			OPENSSL_free(digest);
			return out;
		} catch (...) {
			OPENSSL_free(digest);
			throw;
		}
	}

	template<>
	std::vector<uint8_t> Hasher::value() {
		EVP_DigestFinal_ex(context, digest, &digestLength);
		EVP_MD_CTX_destroy(context);
		alive = false;
		try {
			std::vector out(digest, digest + digestLength);
			OPENSSL_free(digest);
			return out;
		} catch (...) {
			OPENSSL_free(digest);
			throw;
		}
	}

	Hasher::operator bool() const {
		return alive;
	}

	std::string generateSecret(size_t count) {
		std::stringstream ss;
		ss.imbue(std::locale("C"));
		std::random_device rng;
		std::mt19937_64 prng(rng());
		for (size_t i = 0; i < count; ++i)
			ss << std::hex << prng();
		INFO("Secret: {}", ss.str());
		return ss.str();
	}
}
