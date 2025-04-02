#pragma once

namespace Game3 {
	struct SemanticVersion {
		int major;
		int minor;
		int patch;

		auto operator<=>(const SemanticVersion &) const = default;
	};
}
