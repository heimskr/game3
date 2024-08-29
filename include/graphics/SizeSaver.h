#pragma once

namespace Game3 {
	struct RendererContext;

	class SizeSaver {
		public:
			explicit SizeSaver(const RendererContext &);
			SizeSaver(const SizeSaver &) = delete;
			SizeSaver(SizeSaver &&) noexcept;

			~SizeSaver();

			SizeSaver & operator=(const SizeSaver &) = delete;
			SizeSaver & operator=(SizeSaver &&) noexcept;

		private:
			const RendererContext *context = nullptr;
	};
}
