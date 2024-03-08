#pragma once

#include <cstddef>

namespace Game3 {
	class PipeWrapper {
		public:
			int fds[2]{};

			PipeWrapper();
			~PipeWrapper();

			int operator[](size_t) const;

			void close();
			void release();

			inline int readEnd()  const { return fds[0]; }
			inline int writeEnd() const { return fds[1]; }

		private:
			bool isOpen = false;
	};
}
