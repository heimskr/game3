#pragma once

#include <ostream>
#include <string>

namespace Game3 {
	struct HTTP {
		static std::string get(const std::string &url);
		static std::ostream & get(const std::string &url, std::ostream &);
	};
}
