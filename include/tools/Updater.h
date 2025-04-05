#pragma once

#include "threading/Promise.h"

#include <memory>
#include <string>

namespace Game3 {
	class Updater;
	using UpdaterPtr = std::shared_ptr<Updater>;

	class Updater: public std::enable_shared_from_this<Updater> {
		public:
			static UpdaterPtr make();
			static UpdaterPtr make(std::string domain);

			/** Returns false if it was determined that the update shouldn't be installed. */
			Ref<Promise<bool, std::string>> updateFetch();

			/** Returns false if it was determined that the update shouldn't be installed. */
			bool updateLocal(std::string raw_zip);

			bool mayUpdate();

			static std::size_t getLocalHash();

			std::string fetchHash();

			std::string getURL(std::string_view extension) const;

			/** Returns true iff the local hash and remote hash are different. */
			bool checkHash();

		private:
			enum class State: uint8_t {
				Idle,
				FetchingHash,
				FetchingZip,
			};

			Updater();
			Updater(std::string domain);

			std::string domain;
			State state = State::Idle;
	};
}
