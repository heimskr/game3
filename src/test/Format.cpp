#include "test/Testing.h"
#include "util/Format.h"

namespace {
	using namespace Game3;

	class FormatTest: public Test {
		public:
			static Identifier ID() { return "base:test/util/format"; }

			FormatTest() = default;

			void operator()(TestContext &context) {
				context.expectEqual(std::format("{}", std::vector{'a', 'b', 'c'}), "a, b, c");
				context.expectEqual(std::format("{:[]}", std::vector{'a', 'b', 'c'}), "[a][b][c]");
				context.expectEqual(std::format("{:\"\",}", std::vector{'a', 'b', 'c'}), "\"a\",\"b\",\"c\"");
				context.expectEqual(std::format("{:\" , }", std::vector{'a', 'b', 'c'}), "\"a , \"b , \"c ");
				context.expectEqual(std::format("{:[].}", std::vector{'a'}), "[a]");
				context.expectEqual(std::format("{:[].}", std::vector<char>{}), "");
			}
	};

	auto added = addTest<FormatTest>();
}
