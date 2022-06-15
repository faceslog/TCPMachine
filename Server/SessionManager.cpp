#include "SessionManager.hpp"

#include "Session.hpp"

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

	auto res = sessions.insert_or_assign(fd, std::make_unique<Session>(new Session(fd, this)));
	auto insertedPair = res.first;

	// Start the Session
	insertedPair->second->Start();

	guard.unlock();
}

void SessionManager::Remove(const int fd)
{
	guard.lock();

	sessions.at(fd).reset(nullptr);
	sessions.erase(fd);

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