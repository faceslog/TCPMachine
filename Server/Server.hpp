#pragma once

#include <cstdint>
#include <thread>
#include <atomic>

#include "SessionManager.hpp"

namespace TCPMachine {

	class Server {

	public:

		explicit Server(uint16_t port);
		~Server();

		int Start();
		int Stop();

	private:

		// Thread Pool for all sessions
		SessionManager sessions;

		// The advantage of a mutex is that it waits until the lock is open for you
		//  If you want to do nothing if the lock is held, then the atomic variable is great
		std::atomic_bool isRunning;
		std::thread handler;
		
		uint16_t port;

		void HandlerThread();
	};
}
