#include "graphics/RendererContext.h"
#include "graphics/SizeSaver.h"

namespace Game3 {
	SizeSaver::SizeSaver(const RendererContext &context): context(&context) {
		context.pushSize();
	}

	SizeSaver::SizeSaver(SizeSaver &&other) noexcept:
		context(std::exchange(other.context, nullptr)) {}

	SizeSaver::~SizeSaver() {
		if (context)
			context->popSize();
	}

	SizeSaver & SizeSaver::operator=(SizeSaver &&other) noexcept {
		context = std::exchange(other.context, nullptr);
		return *this;
	}
}
