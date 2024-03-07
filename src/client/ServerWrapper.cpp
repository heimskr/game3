#include "Log.h"
#include "client/ServerWrapper.h"
#include "util/Shell.h"

#include <csignal>

namespace Game3 {
	namespace {
		std::filesystem::path KEY_PATH{"localserver.key"};
		std::filesystem::path CERT_PATH{"localserver.crt"};
	}

	void ServerWrapper::run() {
		const bool key_exists  = std::filesystem::exists(KEY_PATH);
		const bool cert_exists = std::filesystem::exists(CERT_PATH);

		if (key_exists != cert_exists)
			throw std::runtime_error("Exactly one of localserver.key, localserver.crt exists (should be both or neither)");

		if (!key_exists && !generateCertificate(KEY_PATH, CERT_PATH))
			throw std::runtime_error("Couldn't generate certificate/private key");
	}

	bool ServerWrapper::generateCertificate(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path) {
		runCommand("/usr/bin/openssl", {
			"req", "-x509", "-newkey", "rsa:4096", "-keyout", key_path.string(), "-out", certificate_path.string(), "-sha256", "-days", "36500", "-nodes", "-subj", "/C=/ST=/L=/O=/OU=/CN=",
		});

		return std::filesystem::exists(certificate_path) && std::filesystem::exists(key_path);
	}
}
