#include "util/Log.h"
#include "util/Util.h"

namespace Game3::Logger {
	int level = 1;

	std::string getTimestamp() {
		return formatTime("%T");
	}

#ifdef LOG_TO_FILE
	struct Closer {
		std::ostream &stream;
		~Closer() {
			std::println(stream);
		}
	};

	std::ofstream & fileStream() {
		static std::ofstream stream;
		static Closer closer{stream};
		if (!stream.is_open()) {
			stream.open("log.txt", std::ios::out | std::ios::app);
			std::println(stream, "=== Game launched on {} ===", formatTime("%A %F %X"));
		}
		return stream;
	}

	std::string stripANSI(std::string_view string) {
		std::string out;
		out.reserve(string.size());
		for (size_t i = 0; i < string.size(); ++i) {
			if (char ch = string[i]; ch == '\x1b') {
				while (i < string.size() && !std::isalpha(string[i])) {
					++i;
				}
			} else {
				out.push_back(ch);
			}
		}
		return out;
	}
#endif
}
