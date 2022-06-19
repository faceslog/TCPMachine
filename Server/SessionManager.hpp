#pragma once

#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <thread>

namespace TCPMachine {

	// Start a Session in a Thread
	class SessionManager {

	public:

		explicit SessionManager(uint8_t nbOfThreads);
		~SessionManager();

		// Start the thread workers
		int StartWorkers();
		// Stop & Join all threads socket on the queue are closed.
		int StopWorkers();

		// Add the socket to the queue to be processed
		void Push(const int fd);

	private:

		uint8_t nbOfThreads;

		// Mutex to prevent writing to session queue at the same time
		std::mutex guardQueue;
		// Mutex to prevent starting while waiting stop to terminate.
		std::mutex guardStartStop;

		// key: FD, val: thread  
		std::vector<std::thread> threadPool;
		// Store key of the terminated threads
		std::queue<int> queue;

		// atomic bool to stop all threads
		std::atomic_bool areRunning;

		// Handler thread will create an run a session
		void WorkerThread();		

		// Take a socket from the queue to process
		int Get();
	};
}