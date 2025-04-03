#pragma once

#include <string>

namespace Game3 {
	class Updater {
		public:
			Updater();

			void update(const std::string &domain);
			void update();
	};
}
