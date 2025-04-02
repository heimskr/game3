#include "graphics/TextRenderer.h"
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

				TextRenderer renderer = TextRenderer::forTesting();
				renderer.initRenderData();

				{
					UString string = "Gangblanc, Gangblanc, give me your answer, do\nI'm half forswonk all for the code of you\nIt won't be a stylish shader\nI can't code up a trader\nBut you'll look cool without the tools\nOf a game engine built for three";
					UString wrapped = string.wrap(renderer, 538, 0.5);
					context.expectEqual("word wrapping", wrapped, "Gangblanc, Gangblanc, \ngive me your answer, \ndo\nI'm half forswonk all \nfor the code of you\nIt won't be a stylish \nshader\nI can't code up a tra-\nder\nBut you'll look cool \nwithout the tools\nOf a game engine bu-\nilt for three");
				}

				{
					UString string = "\n\nGangblanc, Gangblanc, give me your answer, do\nI'm half forswonk all for the code of you\nIt won't be a stylish shader\nI can't code up a trader\nBut you'll look cool without the tools\nOf a game engine built for three\n \n";
					UString wrapped = string.wrap(renderer, 538, 0.5);
					context.expectEqual("word wrapping with extra newlines", wrapped, "\n\nGangblanc, Gangblanc, \ngive me your answer, \ndo\nI'm half forswonk all \nfor the code of you\nIt won't be a stylish \nshader\nI can't code up a tra-\nder\nBut you'll look cool \nwithout the tools\nOf a game engine bu-\nilt for three\n \n");
				}
			}
	};

	static auto added = addTest<UStringTest>();
}
