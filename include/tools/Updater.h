#pragma once

#include <string>

namespace Game3 {
	class Updater {
		public:
			Updater();

			void updateFetch(const std::string &domain);
			void updateLocal(std::string raw_zip);
			void update();
	};
}
