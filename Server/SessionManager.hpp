#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

namespace TCPMachine {

	// Forward Declaration
	class Session;

	using uPtrSession = std::unique_ptr<Session>;

	// Thread Safe Session Management
	class SessionManager {

	public:

		explicit SessionManager();
		~SessionManager();

		void Add(const int fd);
		void Remove(const int fd);
		void BasicRemove(const int fd);

		void TerminateAll();

	private:

		// File Descriptor (int) - std::unique_ptr<Session>
		std::unordered_map<int, uPtrSession> sessions;
		std::mutex guard;
	};
}