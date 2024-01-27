#pragma once

#include "net/Buffer.h"
#include "threading/SharedRecursiveMutex.h"

#include <mutex>
#include <shared_mutex>

#include <nlohmann/json.hpp>

namespace Game3 {
	using DefaultMutex = SharedRecursiveMutex;

	template <typename T, typename M = DefaultMutex>
	struct Lockable: T {
		using Base = T;

		mutable M mutex;

		using T::T;

		Lockable(const Lockable<T, M> &other): T(other.getBase()) {}
		Lockable(const T &other): T(other) {}

		/** Likely data race issue. */
		Lockable(Lockable<T, M> &&other): T(other.getBase()) {}
		Lockable(T &&other): T(other) {}

		Lockable<T, M> & operator=(const Lockable<T, M> &other) {
			if (this == &other)
				return *this;
			auto this_lock = uniqueLock();
			auto other_lock = other.sharedLock();
			T::operator=(other.getBase());
			return *this;
		}

		Lockable<T, M> & operator=(Lockable<T, M> &&other) {
			if (this == &other)
				return *this;
			auto this_lock = uniqueLock();
			auto other_lock = other.uniqueLock();
			T::operator=(std::move(other.getBase()));
			return *this;
		}

		template <typename U>
		Lockable<T, M> & operator=(const U &other) {
			auto lock = uniqueLock();
			T::operator=(other);
			return *this;
		}

		template <typename U>
		Lockable<T, M> & operator=(U &&other) {
			auto lock = uniqueLock();
			T::operator=(std::forward<U>(other));
			return *this;
		}

		template <typename U>
		Lockable<T, M> & unsafeSet(U &&other) {
			T::operator=(std::forward<U>(other));
			return *this;
		}

		inline auto uniqueLock() const { return std::unique_lock(mutex); }
		inline auto sharedLock() const { return std::shared_lock(mutex); }
		inline auto tryUniqueLock() const { return std::unique_lock(mutex, std::try_to_lock); }
		inline auto trySharedLock() const { return std::shared_lock(mutex, std::try_to_lock); }

		inline T & getBase() { return static_cast<T &>(*this); }
		inline const T & getBase() const { return static_cast<const T &>(*this); }

		inline T & getBase(std::shared_lock<std::shared_mutex> &lock) { lock = sharedLock(); return static_cast<T &>(*this); }
		inline T & getBase(std::unique_lock<std::shared_mutex> &lock) { lock = uniqueLock(); return static_cast<T &>(*this); }

		inline const T & getBase(std::shared_lock<std::shared_mutex> &lock) const { lock = sharedLock(); return static_cast<const T &>(*this); }
		inline const T & getBase(std::unique_lock<std::shared_mutex> &lock) const { lock = uniqueLock(); return static_cast<const T &>(*this); }

		template <typename Fn>
		void withShared(const Fn &function) {
			auto lock = sharedLock();
			function();
		}

		template <typename Fn>
		void withUnique(const Fn &function) {
			auto lock = uniqueLock();
			function();
		}

		inline T copyBase() const {
			auto lock = sharedLock();
			return static_cast<T>(*this);
		}
	};

	template <typename T, typename M>
	void to_json(nlohmann::json &json, const Lockable<T, M> &lockable) {
		auto lock = lockable.sharedLock();
		json = lockable.getBase();
	}

	template <typename T, typename M>
	void from_json(const nlohmann::json &json, Lockable<T, M> &lockable) {
		auto lock = lockable.uniqueLock();
		lockable = json.get<T>();
	}

	template <typename T, typename M>
	Buffer & operator+=(Buffer &buffer, const Lockable<T, M> &lockable) {
		auto lock = lockable.sharedLock();
		return buffer += lockable.getBase();
	}

	template <typename T, typename M>
	Buffer & operator<<(Buffer &buffer, const Lockable<T, M> &lockable) {
		auto lock = lockable.sharedLock();
		return buffer << lockable.getBase();
	}

	template <typename T, typename M>
	Buffer & operator>>(Buffer &buffer, Lockable<T, M> &lockable) {
		auto lock = lockable.uniqueLock();
		return buffer >> lockable.getBase();
	}
}

template <typename T>
struct std::formatter<Game3::Lockable<T>> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
    }

	auto format(const auto &lockable, std::format_context &ctx) const {
		auto lock = lockable.sharedLock();
		return std::format_to(ctx.out(), "{}", lockable.getBase());
	}
};
