#pragma once

#include <string>

namespace Game3 {
	class Updater {
		public:
			std::string domain;

			Updater();
			Updater(std::string domain);

			/** Returns false if it was determined that the update shouldn't be installed. */
			bool updateFetch();

			/** Returns false if it was determined that the update shouldn't be installed. */
			bool updateLocal(std::string raw_zip);

			bool mayUpdate();

			std::size_t getLocalHash();

			std::string fetchHash();

			std::string getURL(std::string_view extension) const;

			/** Returns true iff the local hash and remote hash are different. */
			bool checkHash();
	};
}
