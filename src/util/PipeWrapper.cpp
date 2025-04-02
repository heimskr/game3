#include "util/PipeWrapper.h"

#include <stdexcept>
#include <unistd.h>

#ifdef __MINGW32__
#include <fcntl.h>
#include <io.h>
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#endif

namespace Game3 {
	PipeWrapper::PipeWrapper() {

		if (pipe(fds) == -1) {
			throw std::runtime_error("Couldn't create pipe");
		}

		isOpen = true;
	}

	PipeWrapper::~PipeWrapper() {
		close();
	}

	int PipeWrapper::operator[](size_t index) const {
		return fds[index];
	}

	void PipeWrapper::close() {
		if (isOpen) {
			::close(fds[0]);
			::close(fds[1]);
			isOpen = false;
		}
	}

	void PipeWrapper::release() {
		isOpen = false;
	}
}
