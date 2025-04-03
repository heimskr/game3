#include "test/Testing.h"
#include "tools/Updater.h"
#include "util/Log.h"

namespace Game3 {
	class UpdaterTest: public Test {
		public:
			static Identifier ID() { return "base:test/tools/updater"; }

			UpdaterTest() = default;

			void operator()(TestContext &context) {
				if (!context.hasOption("updater")) {
					return;
				}

				Updater updater;
				try {
					updater.update();
					context.pass("updater");
				} catch (const std::exception &err) {
					ERR("Updater failed: {}", err.what());
					context.fail("updater");
				}
			}
	};

	static auto added = addTest<UpdaterTest>();
}
