#pragma once

#include <format>
#include <stdexcept>
#include <unistd.h>
#include <vector>

namespace Game3 {
	class FDWrapper {
		public:
			FDWrapper(int descriptor_):
				descriptor(descriptor_) {}

			~FDWrapper() {
				// Let's hope it actually succeeds.
				::close(descriptor);
			}

		private:
			int descriptor{};
	};

	class CloningFDWrapper {
		public:
			CloningFDWrapper() = default;

			CloningFDWrapper(int descriptor_, const std::vector<int> &to_redirect) {
				init(descriptor_, to_redirect);
			}

			void init(int descriptor_, const std::vector<int> &to_redirect) {
				descriptor = descriptor_;

				for (int redirect: to_redirect) {
					const int clone = dup(redirect);

					if (clone == -1)
						throw std::runtime_error(std::format("Couldn't clone descriptor {} ({})", redirect, errno));

					if (dup2(descriptor, redirect) == -1)
						throw std::runtime_error(std::format("Couldn't dup2({}, {}) ({})", descriptor, redirect, errno));

					redirects.emplace_back(redirect, clone);
				}

				active = true;
			}

			~CloningFDWrapper() {
				close();
			}

			void close() {
				if (!active)
					return;

				active = false;

				for (const auto [redirected, clone]: redirects) {
					if (dup2(clone, redirected) == -1)
						throw std::runtime_error(std::format("Couldn't dup2({}, {}) ({})", clone, redirected, errno));

					if (::close(clone) == -1)
						throw std::runtime_error(std::format("Couldn't close cloned descriptor {} ({})", clone, errno));
				}

				redirects.clear();

				if (::close(descriptor) == -1)
					throw std::runtime_error(std::format("Couldn't close descriptor {} ({})", descriptor, errno));
			}

		private:
			struct Redirect {
				int redirected;
				int clone;

				Redirect(int redirected, int clone):
					redirected(redirected), clone(clone) {}
			};

			bool active = false;
			int descriptor{};
			std::vector<Redirect> redirects;
	};
}
