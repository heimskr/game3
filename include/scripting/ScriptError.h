#pragma once

#ifdef GAME3_ENABLE_SCRIPTING
#include <stdexcept>
#include <string>

#include <v8.h>

namespace Game3 {
	struct ScriptError: std::runtime_error {
		int line = -1;
		int column = -1;

		ScriptError(const std::string &message, int line_ = -1, int column_ = -1):
			std::runtime_error(message), line(line_), column(column_) {}

		ScriptError(const char *message, int line_ = -1, int column_ = -1):
			std::runtime_error(message), line(line_), column(column_) {}
	};
}
#endif
