#include "tools/Updater.h"
#include "util/Log.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#pragma GCC diagnostic pop

#include <format>
#include <sstream>

namespace {
	std::string DEFAULT_DOMAIN = "game3.zip";
}

namespace Game3 {
	Updater::Updater() = default;

	void Updater::update(const std::string &domain) {
		std::string url;
#ifdef __MINGW32__
		url = std::format("https://{}/game3-windows-x86_64.zip", domain);
#elif defined(__linux__)
		url = std::format("https://{}/game3-linux-x86_64.zip", domain);
#else
		throw std::runtime_error("Can't update: unknown platform");
#endif

		curlpp::Easy request;
		request.setOpt(curlpp::options::Url(url));
		std::ostringstream string_stream;
		curlpp::options::WriteStream write_stream(&string_stream);
		request.setOpt(write_stream);
		request.perform();

		std::string zip = std::move(string_stream).str();
	}

	void Updater::update() {
		update(DEFAULT_DOMAIN);
	}
}
