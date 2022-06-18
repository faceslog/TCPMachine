#pragma once

#include <string>
#include <atomic>
#include <thread>

namespace TCPMachine {

	class Session {

	public:

		explicit Session(const int fd);
		~Session();

		// Send a buffer using the current socket, throw std::runtime_error
		void SendData(const char* buffer, uint32_t total_bytes);
		// Receive a buffer using the current socket, throw std::runtime_error
		void RecvData(char* buffer, uint32_t total_bytes);

		// Send an int32_t, throw std::runtime_error
		void SendInt32(const int32_t integer);
		// Recv an int32_t, throw std::runtime_error
		void RecvInt32(int32_t* integer);

		// Send an uint32_t, throw std::runtime_error
		void SendUint32(const uint32_t integer);
		// Recv an uint32_t, throw std::runtime_error
		void RecvUint32(uint32_t* integer);

		// Send a std::string, throw std::runtime_error
		void SendString(const std::string& str);
		// Recv a std::string, throw std::runtime_error, std::bad_alloc
		void RecvString(std::string* str);

		// Send a bool, throw std::runtime_error
		void SendBoolean(const bool value);
		// Recv a bool, throw std::runtime_error
		void RecvBoolean(bool* value);	

		const std::string& GetIpAddress() const;

	private:

		const int fd; 

		// IP:PORT of the client session
		std::string fullIp;

		void SetIpAddress();
	};
}