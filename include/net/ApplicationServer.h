#pragma once

namespace Game3 {
	class ApplicationServer {
		protected:
			ApplicationServer() = default;

		public:
			ApplicationServer(const ApplicationServer &) = delete;
			ApplicationServer(ApplicationServer &&) = delete;

			virtual ~ApplicationServer() = default;

			ApplicationServer & operator=(const ApplicationServer &) = delete;
			ApplicationServer & operator=(ApplicationServer &&) = delete;

			virtual void run() = 0;
			virtual void stop() = 0;
	};
}
