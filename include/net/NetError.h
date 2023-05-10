#pragma once

#include <cstring>
#include <stdexcept>
#include <string>

namespace Game3 {
	class NetError: public std::runtime_error {
		private:
			std::string message;

		public:
			int statusCode;

			explicit NetError(int status_code):
				NetError("Network operation", status_code) {}

			explicit NetError(const std::string &type, int status_code):
				std::runtime_error(type + " failed: " + strerror(status_code)),
				message(type + " failed: " + strerror(status_code)),
				statusCode(status_code) {}

			[[nodiscard]] const char * what() const noexcept override {
				return message.c_str();
			}
	};
}
