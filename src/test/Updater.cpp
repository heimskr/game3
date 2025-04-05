#include "test/Testing.h"
#include "tools/Updater.h"
#include "util/FS.h"
#include "util/Log.h"

namespace Game3 {
	class UpdaterTest: public Test {
		public:
			static Identifier ID() { return "base:test/tools/updater"; }

			UpdaterTest() = default;

			void operator()(TestContext &context) {
				if (!context.hasOption("updater") && !context.hasOption("all")) {
					return;
				}

				auto updater = Updater::make();

				try {
					std::filesystem::path local = "./game3-linux-x86_64.zip";
					if (std::filesystem::exists(local)) {
						updater->updateLocal(readFile(local));
						context.pass("updater local");
					} else {
						updater->updateFetch();
						context.pass("updater remote");
					}
				} catch (const std::exception &err) {
					ERR("Updater failed: {}", err.what());
					context.fail("updater");
				}
			}
	};

	static auto added = addTest<UpdaterTest>();
}
