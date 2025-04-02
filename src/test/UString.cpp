#include "test/Testing.h"
#include "types/UString.h"
#include "util/Log.h"

template <>
struct std::formatter<std::vector<Game3::UStringSpan>> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		for (bool start = true; Game3::UStringSpan span: vector) {
			if (start) {
				start = false;
				std::format_to(ctx.out(), "\"{}\"", span);
			} else {
				std::format_to(ctx.out(), ", \"{}\"", span);
			}
		}

		return ctx.out();
	}
};

namespace Game3 {
	class UStringTest: public Test {
		public:
			static Identifier ID() { return "base:test/types/ustring"; }

			UStringTest() = default;

			void operator()(TestContext &context) {
				{
					UString string = "\nfoo\nbar\n\nbaz\n\n";
					std::vector<UStringSpan> lines = string.getLines();

					if (!context.expectEqual(lines.size(), 6uz)) {
						return;
					}

					context.expectEqual(lines[0], "");
					context.expectEqual(lines[1], "foo");
					context.expectEqual(lines[2], "bar");
					context.expectEqual(lines[3], "");
					context.expectEqual(lines[4], "baz");
					context.expectEqual(lines[5], "");
				}

				{
					UString string = "~~foo~~bar~~~baz~~~~";
					std::vector<UStringSpan> split = UStringSpan(string).split("~~");
					if (!context.expectEqual(split.size(), 6uz)) {
						ERR("Split: {}", split);
						return;
					}
					context.expectEqual(split[0], "");
					context.expectEqual(split[1], "foo");
					context.expectEqual(split[2], "bar");
					context.expectEqual(split[3], "~baz");
					context.expectEqual(split[4], "");
					context.expectEqual(split[5], "");
				}
			}
	};

	static auto added = addTest<UStringTest>();
}
