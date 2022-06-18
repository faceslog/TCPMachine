#include "SessionManager.hpp"

#include <iostream>
#include <stdexcept>

using namespace TCPMachine;

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

void SessionManager::Add(const int socket)
{
}

void SessionManager::CleanUp()
{
}

void SessionManager::TerminateAll()
{
}

void SessionManager::QueueForDeletion(const int id) 
{
}

void SessionManager::HandlerThread(const int fd)
{
	std::cout << "[MANAGER] [THREAD: 0x" << std::this_thread::get_id() << "] : Handler Thread Started" << std::endl;

	// Create a session and handle an automated routine with it

	std::cout << "[MANAGER] [THREAD: 0x" << std::this_thread::get_id() << "] : Handler Thread Stopped" << std::endl;
}