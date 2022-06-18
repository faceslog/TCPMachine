#include "SessionManager.hpp"

#include "Session.hpp"

#include <stdexcept>

using namespace TCPMachine;

SessionManager::SessionManager() : sessions()
{
}

SessionManager::~SessionManager()
{
	TerminateAll();
}

void SessionManager::Add(const int fd)
{
	guard.lock();
	std::unique_lock<std::mutex> lock(guard);

	auto res = sessions.insert_or_assign(fd, std::unique_ptr<Session>(new Session(fd, this)));
	
	// Reference sur la pair que l'on vient d'insert
	auto& insertedPair = res.first;
	// Start the Session
	insertedPair->second->Start();

	// std::mutex::unlock called with std::unique_lock dtor
}

void SessionManager::Remove(const int fd)
{
	std::unique_lock<std::mutex> lock(guard);

	try
	{
		sessions.at(fd)->BasicStop();
		sessions.at(fd).reset(nullptr);
		sessions.erase(fd);
	}
	catch (const std::out_of_range&)
	{
		// Session does not exist ...
	}

	// std::mutex::unlock called with std::unique_lock dtor
}

void SessionManager::BasicRemove(const int fd)
{
	std::unique_lock<std::mutex> lock(guard);

	try
	{
		sessions.at(fd).reset(nullptr);
		sessions.erase(fd);
	}
	catch (const std::out_of_range&)
	{
		// Should never happen !
	}

	// std::mutex::unlock called with std::unique_lock dtor
}

void SessionManager::TerminateAll()
{
	std::unique_lock<std::mutex> lock(guard);

	for (auto& session : sessions)
	{
		session.second->BasicStop();
	}

	// This will call the destructor for each smartpointer freeing the memory allocated for the sessions
	sessions.clear();

	// std::mutex::unlock called with std::unique_lock dtor
}