#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#include "test/Testing.h"
#include "types/UString.h"

namespace Game3 {
	class UStringTest: public Test {
		public:
			static Identifier ID() { return "base:test/types/ustring"; }

			UStringTest() = default;

			void operator()(TestContext &context) {
				UString string = "\nfoo\nbar\n\nbaz\n\n";
				auto lines = string.getLines();

				auto get_line = [&lines](std::size_t i) -> std::string {
					auto [left, right] = lines.at(i);
					UString string;
					while (left != right) {
						string += *left++;
					}
					return string.release();
				};

				context.expectEqual(lines.size(), 6uz);
				context.expectEqual(get_line(0), "");
				context.expectEqual(get_line(1), "foo");
				context.expectEqual(get_line(2), "bar");
				context.expectEqual(get_line(3), "");
				context.expectEqual(get_line(4), "baz");
				context.expectEqual(get_line(5), "");
			}
	};

	static auto added = addTest<UStringTest>();
}
