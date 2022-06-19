#include "SessionManager.hpp"

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "Session.hpp"

using namespace TCPMachine;

SessionManager::SessionManager(uint8_t nbOfThreads) : threadPool(), queue()
{
	this->nbOfThreads = nbOfThreads;
	this->areRunning.store(false);
}

SessionManager::~SessionManager()
{
	if (areRunning.load())
	{
		StopWorkers();
	}
}

void SessionManager::Push(const int socket)
{
	std::unique_lock<std::mutex> lock(guardQueue);

	queue.push(socket);
}

int SessionManager::Get()
{
	std::unique_lock<std::mutex> lock(guardQueue);

	if (queue.empty())
		return -1;

	int fd = queue.front();
	queue.pop();

	return fd;
}

int SessionManager::StartWorkers()
{
	std::unique_lock<std::mutex> lock(guardStartStop);

	if (areRunning.load())
	{
		std::cerr << "[MANAGER] : Worker Threads Already Running !" << std::endl;
		return -1;
	}
		
	if (not areRunning.is_lock_free())
		return -1;

	areRunning.store(true);

	for (int i{ 0 }; i < nbOfThreads; i++)
	{
		threadPool.push_back(std::thread(&SessionManager::WorkerThread, this));
	}
	
	return 0;
}

int SessionManager::StopWorkers()
{
	std::unique_lock<std::mutex> lockA(guardStartStop);

	if (not areRunning.load())
	{
		std::cerr << "[MANAGER] : Worker Threads are Not Running !" << std::endl;
		return -1;
	}

	// ======================================================
	std::cerr << "[MANAGER] : Stopping Worker Threads ..." << std::endl;
	areRunning.store(false);
	for (auto& th : threadPool)
	{
		if (th.joinable())
			th.join();
	}
	std::cerr << "[MANAGER] : Threads Stopped !" << std::endl;

	// ======================================================
	std::cerr << "[MANAGER] : Closing Sockets in Queue ..." << std::endl;
	std::unique_lock<std::mutex> lockB(guardQueue);
	while (not queue.empty())
	{
		int fd = queue.front();
		queue.pop();
		close(fd);
	}
	std::cerr << "[MANAGER] : All Sockets are Closed ..." << std::endl;

	return 0;
}

void SessionManager::WorkerThread()
{
	std::cout << "[MANAGER] [THREAD: 0x" << std::this_thread::get_id() << "] : Worker Thread Started" << std::endl;

	// Take a socket from the queue and process it
	while (areRunning.load())
	{
		int fd = Get();

		if (fd < 0)
		{
			// No socket to use sleeping ...
			std::this_thread::sleep_for(std::chrono::seconds(5));
			continue;
		}
				
		Session bot = Session(fd);
		std::cout << "[MANAGER] [THREAD: 0x" << std::this_thread::get_id() << "] : Connected to: " << bot.GetIpAddress() << std::endl;

		bot.SendString("Hello from Server !");		

		// When the session goes out of scope the dtor will close the socket
	}

	std::cout << "[MANAGER] [THREAD: 0x" << std::this_thread::get_id() << "] : Worker Thread Stopped" << std::endl;
}