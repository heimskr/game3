#include "test/Testing.h"
#include "util/ConstexprHash.h"
#include "util/Log.h"

namespace Game3 {
	class ConstexprHashTest: public Test {
		public:
			static Identifier ID() { return "base:test/util/constexpr_hash"; }

			ConstexprHashTest() = default;

			void operator()(TestContext &context) {
				context.expectEqual(""_fnv, 84696351u);
				context.expectEqual("foo"_fnv, 1632676981u);
			}
	};

	static auto added = addTest<ConstexprHashTest>();
}
