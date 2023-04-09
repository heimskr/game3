#pragma once

namespace Game3 {
	struct Version {
		int major;
		int minor;
		int patch;

		bool operator<(const Version &other) const {
			if (this == &other)
				return false;
			if (major < other.major)
				return true;
			if (major > other.major)
				return false;
			if (minor < other.minor)
				return true;
			if (minor > other.minor)
				return false;
			return patch < other.patch;
		}

		bool operator<=(const Version &other) const {
			if (this == &other)
				return true;
			if (major < other.major)
				return true;
			if (major > other.major)
				return false;
			if (minor < other.minor)
				return true;
			if (minor > other.minor)
				return false;
			return patch <= other.patch;
		}

		bool operator==(const Version &other) const {
			return this == &other || (major == other.major && minor == other.minor && patch == other.patch);
		}
	};
}
