#pragma once

#include <string>
#include <atomic>
#include <thread>

namespace TCPMachine {

	// Forward Declaration for Bidirectional Association 
	// when closing the session we need to remove it from the manager
	class SessionManager;

	class Session {

	public:

		explicit Session(const int fd, SessionManager* manager);
		~Session();

		int Start();

		// Signal the SessionManager that it will stop
		int Stop();
		// Does not signal the manager that it will stop
		int BasicStop();

		// Return the amount of bytes sent if it's a success or return -1 for errors
		int SendData(const char* buffer, uint32_t total_bytes);
		int RecvData(char* buffer, uint32_t total_bytes);

		int SendInt32(const int32_t integer);
		int RecvInt32(int32_t* integer);

		int SendString(const std::string& str);
		int RecvString(std::string* str);

		int SendBoolean(const bool value);
		int RecvBoolean(bool* value);

		const std::string& GetIpAddress();

	private:

		int fd;
		SessionManager* manager;

		std::atomic_bool isActive;
		std::thread handler;

		// IP:PORT of the client session
		std::string fullIp;

		void SetIpAddress();
		void HandlerThread();
	};
}