#pragma once

#include <format>
#include <stdexcept>
#include <unistd.h>

namespace Game3 {
	class FDWrapper {
		public:
			FDWrapper(int descriptor_):
				descriptor(descriptor_) {}

			~FDWrapper() {
				if (::close(descriptor) == -1)
					throw std::runtime_error(std::format("Couldn't close descriptor {} ({})", descriptor, errno));
			}

		private:
			int descriptor{};
	};

	class CloningFDWrapper {
		public:
			CloningFDWrapper(int descriptor_, int to_redirect) {
				init(descriptor_, to_redirect);
			}

			void init(int descriptor_, int to_redirect) {
				clone = dup(redirect);

				if (clone == -1)
					throw std::runtime_error(std::format("Couldn't clone descriptor {} ({})", descriptor, errno));

				if (dup2(descriptor, redirect) == -1)
					throw std::runtime_error(std::format("Couldn't dup2({}, {}) ({})", descriptor, redirect, errno));

				active = true;
			}

			~CloningFDWrapper() {
				close();
			}

			void close() {
				if (!active)
					return;

				active = false;

				if (::close(descriptor) == -1)
					throw std::runtime_error(std::format("Couldn't close descriptor {} ({})", descriptor, errno));

				if (clone == -1)
					return;

				if (dup2(clone, redirect) == -1)
					throw std::runtime_error(std::format("Couldn't dup2({}, {}) ({})", clone, redirect, errno));

				if (::close(clone) == -1)
					throw std::runtime_error(std::format("Couldn't close cloned descriptor {} ({})", clone, errno));
			}

		private:
			bool active = false;
			int descriptor{};
			int redirect{};
			int clone{};
	};
}
