#include "SessionManager.hpp"

#include "Session.hpp"

#include <stdexcept>

using namespace TCPMachine;

SessionManager::SessionManager() : sessions()
{
	guard.unlock();
}

SessionManager::~SessionManager()
{
	TerminateAll();
}

void SessionManager::Add(const int fd)
{
	guard.lock();

	auto res = sessions.insert_or_assign(fd, std::unique_ptr<Session>(new Session(fd, this)));
	
	// Reference sur la pair que l'on vient d'insert
	auto& insertedPair = res.first;
	// Start the Session
	insertedPair->second->Start();

	guard.unlock();
}

void SessionManager::Remove(const int fd)
{
	guard.lock();

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

	guard.unlock();
}

void SessionManager::BasicRemove(const int fd)
{
	guard.lock();

	try
	{
		sessions.at(fd).reset(nullptr);
		sessions.erase(fd);
	}
	catch (const std::out_of_range&)
	{
		// Should never happen !
	}

	guard.unlock();
}

void SessionManager::TerminateAll()
{
	guard.lock();

	for (auto& session : sessions)
	{
		session.second->BasicStop();
	}

	// This will call the destructor for each smartpointer freeing the memory allocated for the sessions
	sessions.clear();

	guard.unlock();
}