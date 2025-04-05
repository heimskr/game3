#pragma once

#include "threading/Promise.h"

#include <ostream>
#include <string>

namespace Game3 {
	struct HTTP {
		static Ref<Promise<std::string, std::string>> get(std::string url);
		/** The stream must remain valid until the promise resolves! */
		static Ref<Promise<void, std::string>> get(std::string url, std::ostream &);
	};
}
