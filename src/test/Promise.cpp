#include "test/Testing.h"
#include "threading/Promise.h"

namespace Game3 {
	class PromiseTest: public Test {
		public:
			static Identifier ID() { return "base:test/threading/promise"; }

			PromiseTest() = default;

			void operator()(TestContext &context) {
				Promise<int, int>::make([](auto resolve, auto) {
					resolve(42);
				})->then([&](int result) {
					context.expectEqual("resolve(42)", result, 42);
				})->oops([&](int) {
					context.fail("resolve(42)");
				})->wait();

				Promise<int, int>::make([](auto, auto reject) {
					reject(64);
				})->then([&](int) {
					context.fail("reject(64)");
				})->oops([&](int result) {
					context.expectEqual("reject(64)", result, 64);
				})->wait();

				context.expect("resolve(100)", Promise<int, std::string>::make([](auto resolve, auto) {
					resolve(100);
				})->get(), 100);

				context.unexpect("reject(\"oof\")", Promise<int, std::string>::make([](auto, auto reject) {
					reject("oof");
				})->get(), std::string("oof"));

				Promise<int, int>::make([](auto resolve, auto) {
					resolve(-1);
				})->then([&](int result) {
					context.expectEqual("resolve(-1)", result, -1);
				});
			}
	};

	static auto added = addTest<PromiseTest>();
}
