#pragma once

#include <iostream>

#include "net/Sock.h"

namespace Game3 {
	class SocketBuffer: public std::streambuf {
		private:
			Sock *source;
			char *buffer;
			size_t bufferSize;
			size_t putbackSize;

		protected:
			virtual std::streambuf::int_type  overflow(std::streambuf::int_type)      override;
			virtual std::streamsize             xsputn(const char *, std::streamsize) override;
			virtual std::streambuf::int_type underflow()                              override;

		public:
			SocketBuffer(Sock *source_, size_t buffer_size = 64, size_t putback_size = 4);
			~SocketBuffer();

			/** Closes the underlying socket connection. */
			void close();
	};
}
