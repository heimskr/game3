#pragma once

namespace Game3 {
	class UString;

	class Autocompleter {
		public:
			virtual ~Autocompleter() = default;

			virtual void autocomplete(const UString &) = 0;
	};
}
