#include "test/Testing.h"
#include "types/UString.h"

namespace Game3 {
	class UStringTest: public Test {
		public:
			static Identifier ID() { return "base:test/types/ustring"; }

			UStringTest() = default;

			void operator()(TestContext &context) {
				UString string = "\nfoo\nbar\n\nbaz\n\n";
				std::vector<UStringSpan> lines = string.getLines();

				if (context.expectEqual(lines.size(), 6uz)) {
					context.expectEqual(lines[0], "");
					context.expectEqual(lines[1], "foo");
					context.expectEqual(lines[2], "bar");
					context.expectEqual(lines[3], "");
					context.expectEqual(lines[4], "baz");
					context.expectEqual(lines[5], "");
				}
			}
	};

	static auto added = addTest<UStringTest>();
}
