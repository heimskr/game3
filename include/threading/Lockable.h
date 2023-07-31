#pragma once

#include <mutex>
#include <shared_mutex>

#include <nlohmann/json.hpp>

#include "net/Buffer.h"

namespace Game3 {
	template <typename T, typename M = std::shared_mutex>
	struct Lockable: T {
		using T::T;

		Lockable(const Lockable<T> &other): T(other.getBase()) {}
		Lockable(const T &other): T(other) {}

		/** Likely data race issue. */
		Lockable(Lockable<T> &&other): T(other.getBase()) {}
		Lockable(T &&other): T(other) {}

		Lockable<T> & operator=(const Lockable<T> &other) {
			auto this_lock = uniqueLock();
			auto other_lock = const_cast<Lockable<T> &>(other).sharedLock();
			T::operator=(other.getBase());
			return *this;
		}

		Lockable<T> & operator=(Lockable<T> &&other) {
			auto this_lock = uniqueLock();
			auto other_lock = other.uniqueLock();
			T::operator=(std::move(other.getBase()));
			return *this;
		}

		template <typename U>
		Lockable<T> & operator=(const U &other) {
			auto lock = uniqueLock();
			T::operator=(other);
			return *this;
		}

		template <typename U>
		Lockable<T> & operator=(U &&other) {
			auto lock = uniqueLock();
			T::operator=(std::forward<U>(other));
			return *this;
		}

		M mutex;

		inline auto uniqueLock() { return std::unique_lock(mutex); }
		inline auto sharedLock() { return std::shared_lock(mutex); }

		inline T & getBase() { return static_cast<T &>(*this); }
		inline const T & getBase() const { return static_cast<const T &>(*this); }

		inline T copyBase() {
			auto lock = sharedLock();
			return static_cast<T>(*this);
		}
	};

	template <typename T>
	void to_json(nlohmann::json &json, const Lockable<T> &lockable) {
		auto lock = const_cast<Lockable<T> &>(lockable).sharedLock();
		json = lockable.getBase();
	}

	template <typename T>
	void from_json(const nlohmann::json &json, Lockable<T> &lockable) {
		auto lock = lockable.uniqueLock();
		lockable = json.get<T>();
	}

	template <typename T>
	std::ostream & operator<<(std::ostream &os, const Lockable<T> &lockable) {
		auto lock = const_cast<Lockable<T> &>(lockable).sharedLock();
		return os << lockable.getBase();
	}

	template <typename T>
	Buffer & operator+=(Buffer &buffer, const Lockable<T> &lockable) {
		auto lock = const_cast<Lockable<T> &>(lockable).sharedLock();
		return buffer += lockable.getBase();
	}

	template <typename T>
	Buffer & operator<<(Buffer &buffer, const Lockable<T> &lockable) {
		auto lock = const_cast<Lockable<T> &>(lockable).sharedLock();
		return buffer << lockable.getBase();
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, Lockable<T> &lockable) {
		auto lock = lockable.uniqueLock();
		return buffer >> lockable.getBase();
	}
}
