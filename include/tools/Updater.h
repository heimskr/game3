#pragma once

#include <string>

namespace Game3 {
	class Updater {
		public:
			Updater();

			/** Returns false if it was determined that the update shouldn't be installed. */
			bool updateFetch(const std::string &domain);

			/** Returns false if it was determined that the update shouldn't be installed. */
			bool updateLocal(std::string raw_zip);

			/** Returns false if it was determined that the update shouldn't be installed. */
			bool update();

			bool mayUpdate();

			std::size_t getLocalHash();
	};
}
