#pragma once

#include "data/Identifier.h"

#include <format>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace Game3 {
	class Game;
	using GamePtr = std::shared_ptr<Game>;

	class TestContext {
		public:
			GamePtr game;

			using Reporter = std::function<void(std::string_view)>;
			TestContext(Reporter pass, Reporter fail, std::string_view option_string, GamePtr game);

			void pass(std::string_view);
			void fail(std::string_view);
			bool report(std::string_view, bool passed);
			bool hasOption(const std::string &option) const;

			template <typename T, typename U>
			bool expectEqual(const T &actual, const U &expected) {
				if (actual == expected) {
					pass(std::format("expect \"{}\"", expected));
					return true;
				} else {
					fail(std::format("expect actual \"{}\" == expected \"{}\"", actual, expected));
					return false;
				}
			}

			template <typename T, typename U>
			bool expectEqual(std::string_view name, T &&actual, U &&expected) {
				return report(name, std::forward<T>(actual) == std::forward<U>(expected));
			}

		private:
			Reporter passReporter;
			Reporter failReporter;
			std::unordered_set<std::string> options;
	};

	class Test {
		public:
			virtual ~Test() = default;
			virtual void operator()(TestContext &) = 0;
			virtual void cleanup();
	};

	using TestPtr = std::shared_ptr<Test>;

	class Tests: public std::map<Identifier, TestPtr> {
		public:
			using Base = std::map<Identifier, TestPtr>;
			using Base::Base;

			bool runAll(std::string_view options = {}, const GamePtr & = {});

			static Tests & get();
	};

	template <typename T, typename... Args>
	auto addTest(Args &&...args) {
		return Tests::get()[T::ID()] = std::make_shared<T>(std::forward<Args>(args)...);
	}
}
