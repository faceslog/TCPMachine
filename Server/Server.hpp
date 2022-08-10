#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <thread>

#include "SessionManager.hpp"

namespace TCPMachine {

	class Server {

	public:

		// Port of the server & nb of threads to handle a sessions at the same time
		explicit Server(uint16_t port, uint8_t nbWorkers);
		~Server();

		// Start the listener in a new thread - Total threads: nbWorkers + 1
		int Start();
		int Stop();

	private:

		// Thread pool to manage sessions
		SessionManager sessions;

		// To prevent calling Start before Stop finishes & vice versa
		std::mutex guardStartStop;
		// To Stop the listener thread
		std::atomic_bool isRunning;
		// ListenerThread
		std::thread handle;

		// Server Port
		uint16_t port;

		// Create a socket and listen for clients
		void ListenerThread();

		// Return the listen socket or -1 for errors
		int CreateListenSock();
	};
}
