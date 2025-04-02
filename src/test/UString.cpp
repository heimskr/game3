#include "test/Testing.h"
#include "types/UString.h"
#include "util/Format.h"
#include "util/Log.h"

namespace Game3 {
	class UStringTest: public Test {
		public:
			static Identifier ID() { return "base:test/types/ustring"; }

			UStringTest() = default;

			void operator()(TestContext &context) {
				{
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

				{
					UString string = "~~foo~~bar~~~baz~~~~";
					std::vector<UStringSpan> split = UStringSpan(string).split("~~");
					if (context.expectEqual(split.size(), 6uz)) {
						context.expectEqual(split[0], "");
						context.expectEqual(split[1], "foo");
						context.expectEqual(split[2], "bar");
						context.expectEqual(split[3], "~baz");
						context.expectEqual(split[4], "");
						context.expectEqual(split[5], "");
					}
				}

				{
					UString string = "~~";
					std::vector<UStringSpan> split = UStringSpan(string).split("~~");
					if (context.expectEqual(split.size(), 2uz)) {
						context.expectEqual(split[0], "");
						context.expectEqual(split[1], "");
					}
				}
			}
	};

	static auto added = addTest<UStringTest>();
}
