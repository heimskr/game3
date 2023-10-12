#pragma once

#include <string>

namespace Game3 {
	std::string demangle(const char *);
}

#define DEMANGLE(x) ::Game3::demangle(typeid(x).name())