#include "test/Testing.h"
#include "threading/Promise.h"

namespace Game3 {
	class PromiseTest: public Test {
		public:
			static Identifier ID() { return "base:test/threading/promise"; }

			PromiseTest() = default;

			void operator()(TestContext &context) {
				Promise<int>::make([](auto resolve) {
					resolve(42);
				})->then([&](int result) {
					context.expectEqual("resolve(42)", result, 42);
				})->oops([&](std::exception_ptr) {
					context.fail("resolve(42)");
				})->wait();

				Promise<int>::make([](auto) {
					throw 42;
				})->then([&](int) {
					context.fail("reject(42)");
				})->oops([&](std::exception_ptr result) {
					try {
						std::rethrow_exception(std::move(result));
					} catch (int value) {
						context.expectEqual("reject(42)", value, 42);
					}
				})->wait();
			}
	};

	static auto added = addTest<PromiseTest>();
}
