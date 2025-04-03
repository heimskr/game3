#include "test/Testing.h"
#include "util/Log.h"
#include "util/Util.h"

namespace {
	constexpr const char * plural(std::size_t count) {
		return count == 1? "test" : "tests";
	}
}

namespace Game3 {
	TestContext::TestContext(Reporter pass, Reporter fail, std::string_view option_string, GamePtr game):
		game(std::move(game)),
		passReporter(std::move(pass)),
		failReporter(std::move(fail)) {
			std::vector<std::string> vector = split<std::string>(option_string, ",");
			options = {std::move_iterator(vector.begin()), std::move_iterator(vector.end())};
		}

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

	bool TestContext::hasOption(const std::string &option) const {
		return options.contains(option);
	}

	Tests & Tests::get() {
		static Tests tests;
		return tests;
	}

	void Test::cleanup() {}

	bool Tests::runAll(std::string_view options, const GamePtr &game) {
		std::size_t good = 0;
		std::size_t bad = 0;

		TestContext context([&](std::string_view test_name) {
			SUCCESS("Passed \e[1m{}\e[22m", test_name);
			++good;
		}, [&](std::string_view test_name) {
			ERR("Failed \e[1m{}\e[22m", test_name);
			++bad;
		}, options, game);

		for (const auto &[id, test]: *this) {
			const std::size_t old_good = good;
			const std::size_t old_bad = bad;

			try {
				(*test)(context);
			} catch (const std::exception &err) {
				good = old_good;
				bad = old_bad;
				context.fail(id.str());
				ERR("Test {} caught exception: {}", id, err.what());
			} catch (...) {
				good = old_good;
				bad = old_bad;
				context.fail(id.str());
				ERR("Test {} caught exception", id);
			}

			test->cleanup();
		}

		if (good == 0 && bad == 0) {
			WARN("No tests were run.");
		} else if (good > 0 && bad == 0) {
			if (good == 1) {
				SUCCESS("Summary: passed 1 test.");
			} else {
				SUCCESS("Summary: passed all {} tests.", good);
			}
		} else if (bad > 0 && good == 0) {
			if (bad == 1) {
				ERR("Summary: failed \e[31m1\e[39m test.");
			} else {
				ERR("Summary: failed all \e[31m{}\e[39m tests.", bad);
			}
		} else {
			WARN("Summary: passed \e[32m{}\e[39m {} and failed \e[31m{}\e[39m {}.", good, plural(good), bad, plural(bad));
		}

		return bad == 0;
	}
}
