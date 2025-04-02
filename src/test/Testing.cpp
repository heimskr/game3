#include "test/Testing.h"
#include "util/Log.h"

namespace {
	constexpr const char * plural(std::size_t count) {
		return count == 1? "test" : "tests";
	}
}

namespace Game3 {
	TestContext::TestContext(Reporter pass, Reporter fail, GamePtr game):
		game(std::move(game)),
		passReporter(std::move(pass)),
		failReporter(std::move(fail)) {}

	void TestContext::pass(std::string_view test_name) {
		passReporter(test_name);
	}

	void TestContext::fail(std::string_view test_name) {
		failReporter(test_name);
	}

	bool TestContext::report(std::string_view test_name, bool passed) {
		if (passed) {
			pass(test_name);
		} else {
			fail(test_name);
		}

		return passed;
	}

	Tests & Tests::get() {
		static Tests tests;
		return tests;
	}

	bool Tests::runAll(const GamePtr &game) {
		std::size_t good = 0;
		std::size_t bad = 0;

		TestContext context([&](std::string_view test_name) {
			SUCCESS("Passed \e[1m{}\e[22m", test_name);
			++good;
		}, [&](std::string_view test_name) {
			ERR("Failed \e[1m{}\e[22m", test_name);
			++bad;
		}, game);

		for (const auto &[id, test]: *this) {
			(*test)(context);
		}

		if (good == 0 && bad == 0) {
			WARN("No tests were run.");
		} else if (good > 0 && bad == 0) {
			SUCCESS("Summary: passed {} {}.", good, plural(good));
		} else if (bad > 0 && good == 0) {
			ERR("Summary: failed \e[31m{}\e[39m {}.", bad, plural(bad));
		} else {
			WARN("Summary: passed \e[32m{}\e[39m {} and failed \e[31m{}\e[39m {}.", good, plural(good), bad, plural(bad));
		}

		return bad == 0;
	}
}
