#include "net/CertGen.h"

#include <openssl/pem.h>
#include <openssl/x509.h>

#include <cstdio>
#include <format>
#include <memory>

namespace Game3 {
	void generateCertPair(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path) {
		// Credit: https://gist.github.com/nathan-osman/5041136

		std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(EVP_RSA_gen(4096), EVP_PKEY_free);

		if (!pkey) {
			throw std::runtime_error("Unable to create EVP_PKEY");
		}

		std::unique_ptr<X509, decltype(&X509_free)> x509(X509_new(), X509_free);

		if (!x509) {
			throw std::runtime_error("Unable to create X509");
		}

		ASN1_INTEGER_set(X509_get_serialNumber(x509.get()), 1);
		X509_gmtime_adj(X509_get_notBefore(x509.get()), 0);
		X509_gmtime_adj(X509_get_notAfter(x509.get()), 3153600000); // 100 years
		X509_set_pubkey(x509.get(), pkey.get());

		if (!X509_sign(x509.get(), pkey.get(), EVP_sha256())) {
			throw std::runtime_error("Failed to sign certificate");
		}

		FILE *pkey_file = fopen(key_path.string().c_str(), "wb");

		if (!pkey_file) {
			throw std::runtime_error(std::format("Couldn't open {} for writing", key_path.string().c_str()));
		}

		if (!PEM_write_PrivateKey(pkey_file, pkey.get(), nullptr, nullptr, 0, nullptr, nullptr)) {
			fclose(pkey_file);
			throw std::runtime_error(std::format("Couldn't write private key to {}", key_path.string().c_str()));
		}

		fclose(pkey_file);

		FILE *x509_file = fopen(certificate_path.string().c_str(), "wb");
		if (!x509_file) {
			throw std::runtime_error(std::format("Couldn't open {} for writing", certificate_path.string().c_str()));
		}

		if (!PEM_write_X509(x509_file, x509.get())) {
			fclose(x509_file);
			throw std::runtime_error(std::format("Couldnt' write certificate to {}", certificate_path.string().c_str()));
		}

		fclose(x509_file);
	}
}
