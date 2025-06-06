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

	Ref<Promise<std::string>> HTTP::get(std::string url) {
		return Promise<std::string>::now([url = std::move(url)](auto resolve) {
			std::ostringstream stream;
			get(url, stream)->wait();
			resolve(std::move(stream).str());
		});
	}

	Ref<Promise<void>> HTTP::get(std::string url, std::ostream &stream) {
		return Promise<void>::now([url = std::move(url), &stream](auto resolve) {
			maybeInitializeCurl();
			curlpp::Easy request;
			request.setOpt(curlpp::options::Url(url));
			request.setOpt(curlpp::options::WriteStream(&stream));
			request.setOpt(curlpp::options::FailOnError(true));
#ifdef __MINGW32__
			request.setOpt(curlpp::options::SslOptions(CURLSSLOPT_NATIVE_CA));
#endif
			request.perform();
			resolve();
		});
	}
}