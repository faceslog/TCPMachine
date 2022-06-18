#pragma once

#include <mutex>
#include <atomic>
#include <unordered_map>
#include <queue>
#include <thread>

namespace TCPMachine {

	// Start a Session in a Thread
	class SessionManager {

	public:

		explicit SessionManager();
		~SessionManager();

		// Create a new thread to handle a session
		void Add(const int fd);
		// Join & Clean terminated threads
		void CleanUp();
		// Stop & Join all threads.
		void TerminateAll();

	private:

		std::mutex guard;

		// key: FD, val: thread  
		std::unordered_map<int, std::thread> threadPool;
		// Store key of the terminated threads
		std::queue<int> deletionQueue;
		// atomic bool to stop all threads
		std::atomic_bool areRunning;
		// Add a thread to the queue for deletion
		void QueueForDeletion(const int fd);

		// Handler thread will create an run a session
		void HandlerThread(const int fd);		
	};
}