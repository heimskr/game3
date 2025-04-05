#include "net/HTTP.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#pragma GCC diagnostic pop

#include <sstream>

namespace Game3 {
	static void maybeInitializeCurl() {
		static bool initialized = false;
		if (!initialized) {
			curlpp::initialize();
			initialized = true;
		}
	}

	std::string HTTP::get(const std::string &url) {
		std::ostringstream stream;
		get(url, stream);
		return std::move(stream).str();
	}

	std::ostream & HTTP::get(const std::string &url, std::ostream &stream) {
		maybeInitializeCurl();
		curlpp::Easy request;
		request.setOpt(curlpp::options::Url(url));
		request.setOpt(curlpp::options::WriteStream(&stream));
		request.setOpt(curlpp::options::FailOnError(true));
#ifdef __MINGW32__
		request.setOpt(curlpp::options::SslOptions(CURLSSLOPT_NATIVE_CA));
#endif
		request.perform();
		return stream;
	}
}