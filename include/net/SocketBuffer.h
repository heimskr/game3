#pragma once

#include <iostream>
#include <memory>

namespace Game3 {
	class Sock;

	class SocketBuffer: public std::streambuf {
		private:
			std::shared_ptr<Sock> source;
			char *buffer;
			size_t bufferSize;
			size_t putbackSize;

		protected:
			virtual std::streambuf::int_type  overflow(std::streambuf::int_type)      override;
			virtual std::streamsize             xsputn(const char *, std::streamsize) override;
			virtual std::streambuf::int_type underflow()                              override;

		public:
			SocketBuffer(std::shared_ptr<Sock>, size_t buffer_size = 64, size_t putback_size = 4);
			~SocketBuffer();

			/** Closes the underlying socket connection. */
			void close();
	};
}
