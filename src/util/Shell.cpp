#include "Log.h"
#include "util/Shell.h"

#include <array>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <thread>

namespace Game3 {
	PipeWrapper::PipeWrapper() {
		if (pipe(fds) == -1)
			throw std::runtime_error("Couldn't create pipe");
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

	CommandOutput runCommand(const std::string &path, std::span<const std::string> args, std::chrono::microseconds timeout, int signal_on_timeout) {

		static thread_local PipeWrapper control_pipe;
		PipeWrapper stdout_pipe;
		PipeWrapper stderr_pipe;

		std::condition_variable condition_variable;
		std::mutex mutex;
		bool child_quit = false;

		int child = -1;

		std::thread thread([&] {
			auto until = std::chrono::system_clock::now() + timeout;
			while (!child_quit) {
				std::unique_lock lock(mutex);
				condition_variable.wait_until(lock, until);
				if (until <= std::chrono::system_clock::now()) {
					char dummy{};

					if (write(control_pipe[1], &dummy, 1) == -1)
						throw std::runtime_error("Couldn't write to control pipe");

					if (!child_quit && signal_on_timeout != 0 && child != -1)
						kill(child, signal_on_timeout);

					return;
				}
			}
		});

		for (int i: {0, 1}) {
			fcntl(control_pipe[i], F_SETFL, fcntl(control_pipe[i], F_GETFL) | O_NONBLOCK);
			fcntl(stdout_pipe[i],  F_SETFL, fcntl(stdout_pipe[i],  F_GETFL) | O_NONBLOCK);
			fcntl(stderr_pipe[i],  F_SETFL, fcntl(stderr_pipe[i],  F_GETFL) | O_NONBLOCK);
		}

		static struct sigaction action{};

		action.sa_handler = +[](int) {
			const int saved_errno = errno;
			write(control_pipe[1], "", 1);
			errno = saved_errno;
		};

		sigaction(SIGCHLD, &action, nullptr);

		child = fork();

		if (child == -1)
			throw std::runtime_error("Couldn't fork");

		if (child == 0) {
			std::vector<char *> cstrings{const_cast<char *>(path.c_str())};
			cstrings.reserve(args.size() + 2);

			for (const std::string &arg: args)
				cstrings.push_back(const_cast<char *>(arg.data()));

			cstrings.push_back(nullptr);

			control_pipe.release();
			stdout_pipe.release();
			stderr_pipe.release();

			dup2(stdout_pipe[1], STDOUT_FILENO);
			dup2(stderr_pipe[1], STDERR_FILENO);
			close(stdout_pipe[1]);
			close(stderr_pipe[1]);
			if (execv(path.c_str(), cstrings.data()) == -1)
				throw std::runtime_error("execv failed: " + std::to_string(errno));
			return {};
		}

		int status{};
		fd_set fds{};
		timeval tv{};

		tv.tv_sec  = timeout.count() / 1'000'000;
		tv.tv_usec = timeout.count() % 1'000'000;

		std::stringstream stdout_stream, stderr_stream;

		FD_SET(control_pipe[0], &fds);
		FD_SET(stdout_pipe[0], &fds);
		FD_SET(stderr_pipe[0], &fds);

		std::array<char, 4096> buffer{};
		ssize_t bytes_read{};

		auto fds_copy = fds;
		auto tv_copy = tv;

		while (select(3, &fds_copy, nullptr, nullptr, &tv_copy) != -1 || errno == EINTR) {
			if (FD_ISSET(control_pipe[0], &fds)) {
				char dummy;
				if (0 < read(control_pipe[0], &dummy, 1)) {
					child_quit = true;
					condition_variable.notify_all();
				}
			}

			if (FD_ISSET(stdout_pipe[0], &fds)) {
				while (0 < (bytes_read = read(stdout_pipe[0], buffer.data(), buffer.size())))
					stdout_stream.write(buffer.data(), bytes_read);
			}

			if (FD_ISSET(stderr_pipe[0], &fds)) {
				while (0 < (bytes_read = read(stderr_pipe[0], buffer.data(), buffer.size())))
					stderr_stream.write(buffer.data(), bytes_read);
			}

			if (int wait_result = waitpid(child, &status, WNOHANG); wait_result != -1) {
				child_quit = true;
				condition_variable.notify_all();
				break;
			}

			fds_copy = fds;
			tv_copy = tv;
		}

		thread.join();
		return {stdout_stream.str(), stderr_stream.str()};
	}
}
