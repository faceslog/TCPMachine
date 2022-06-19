#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <thread>

#include "SessionManager.hpp"

// Forward Declaration
struct sockaddr_in6;

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
		
		// 1) Return 0 for success -1 for errors and close the socket
		int CreateReuseableFd(int* fd);
		// 2) Return 0 for success -1 for errors and close the socket
		int SetSocketFlags(int* fd);
		// 3) Return 0 for success -1 for errors and close the socket
		int BindSocket(int* fd, sockaddr_in6* addr);
		// 4) Return 0 for success -1 for errors and close the socket
		int ListenSocket(int* fd);
	};
}
