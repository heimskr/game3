#pragma once

#include "graphics/Rectangle.h"
#include "graphics/SizeSaver.h"

#include <vector>

namespace Game3 {
	class ScissorStack;
	struct RendererContext;

	class ScissorSaver {
		public:
			ScissorSaver(ScissorStack &, SizeSaver &&);
			ScissorSaver(ScissorSaver &) = delete;
			ScissorSaver(ScissorSaver &&) noexcept;
			~ScissorSaver();
			ScissorSaver & operator=(const ScissorSaver &) = delete;
			ScissorSaver & operator=(ScissorSaver &&) noexcept;

		private:
			ScissorStack *scissorStack;
			SizeSaver sizeSaver;
	};

	class ScissorStack {
		public:
			struct Item {
				Rectangle rectangle;
				bool doViewport;
				bool doScissor;

				Item(const Rectangle &rectangle, bool do_viewport = false, bool do_scissor = true):
					rectangle(rectangle), doViewport(do_viewport), doScissor(do_scissor) {}

				void apply(int base_height) const;
			};

			ScissorStack();

			inline const Rectangle & getBase() const { return base; }
			void setBase(const Rectangle &);
			Item getTop() const;
			const Rectangle & pushRelative(const Item &);
			const Rectangle & pushAbsolute(const Item &);
			[[nodiscard]] ScissorSaver pushRelative(const Rectangle &, const RendererContext &);
			[[nodiscard]] ScissorSaver pushAbsolute(const Rectangle &, const RendererContext &);
			void pop();
			void debug() const;

		private:
			Rectangle base;
			std::vector<Item> stack;
	};
}
