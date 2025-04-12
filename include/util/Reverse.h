#pragma once

namespace Game3 {
	// Credit: https://stackoverflow.com/a/28139075/227663
	template <typename T>
	concept Reversible = requires(T t) {
		t.rbegin();
		t.rend();
	};

	template <Reversible T>
	struct reverse {
		T &iterable;
		reverse() = delete;
		reverse(T &iterable_): iterable(iterable_) {}
	};

	template <Reversible T>
	auto begin(reverse<T> r) {
		return std::rbegin(r.iterable);
	}

	template <Reversible T>
	auto end(reverse<T> r) {
		return std::rend(r.iterable);
	}
}
