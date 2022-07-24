#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <cstdint>
#include <string>

namespace TCPMachine {
	
	class ClientSocket {

	public:

		// throw std::runtime error
		explicit ClientSocket(std::string host, std::string port);
		~ClientSocket();

		// Send a buffer, throw std::runtime_error
		void SendData(const char* buffer, uint32_t total_bytes);
		// Receive a buffer, throw std::runtime_error
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
				
	private:

		const std::string host;
		const std::string port;

		SOCKET connSocket;
		
		// Called by the CTOR, return 0 if it succeed or -1 if it failed
		int InitSocket();

		// int ShutDownSending();
	};
}
