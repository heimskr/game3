#include "util/Demangle.h"

#ifdef __GNUG__
#include <cstdlib>
#include <memory>

#include <cxxabi.h>

namespace Game3 {
	std::string demangle(const char *name) {
		int status = INT_MIN;
		std::unique_ptr<char[], void(*)(void *)> res(abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);
		return status == 0? res.get() : name;
	}
}

#else

namespace Game3 {
	std::string demangle(const char *name) {
		return name;
	}
}

#endif
