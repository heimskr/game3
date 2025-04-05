#include <vector>

template <typename T>
struct std::hash<std::vector<T>> {
	static std::size_t operator()(const std::vector<T> &vec) {
		// Credit: https://stackoverflow.com/a/27216842
		std::size_t seed = vec.size();
		std::hash<T> hasher{};

		for (const auto &item: vec) {
			std::size_t subhash = hasher(item);
			subhash = ((subhash >> 16) ^ subhash) * 0x45d9f3b;
			subhash = ((subhash >> 16) ^ subhash) * 0x45d9f3b;
			subhash = (subhash >> 16) ^ subhash;
			seed ^= subhash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		return seed;
	}
};
