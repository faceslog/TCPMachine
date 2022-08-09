#include "Session.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>

using namespace TCPMachine;

// ======================= PUBLIC: =======================

Session::Session(const int fd) : fd(fd)
{
	SetIpAddress();
}

Session::~Session()
{
	std::cout << "[SESSION] Session Destroyed" << std::endl;
	// To avoid to close twice the socket we will do it in the DTOR
	close(fd);
}

const std::string& Session::GetIpAddress() const
{
	return fullIp;
}

// ======================= PRIVATE: =======================

void Session::SetIpAddress()
{
	socklen_t len;
	uint16_t port{ 0 };
	struct sockaddr_storage addr {};

	len = sizeof addr;
	getpeername(fd, (struct sockaddr*)&addr, &len);

	char _ip[INET6_ADDRSTRLEN];

	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET)
	{
		auto* s = (struct sockaddr_in*)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, _ip, sizeof(_ip));
	}
	else
	{
		// AF_INET6
		auto* s = (struct sockaddr_in6*)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, _ip, sizeof(_ip));
	}

	fullIp = std::string(_ip) + ':' + std::to_string(port);
}

void Session::SendData(const char* buffer, uint32_t total_bytes)
{
	uint32_t bytes_sent = 0;

	while (bytes_sent < total_bytes)
	{						
		int32_t iResult = static_cast<int32_t>(send(fd, buffer + bytes_sent, total_bytes - bytes_sent, 0));
	
		if (iResult < 0)
			throw std::runtime_error("Failed to send data");

		// iResult here is always >= 0 meaning we can add it to an unsigned int
		bytes_sent += iResult;
	}

	if (bytes_sent != total_bytes)
		throw std::runtime_error("Sent " + std::to_string(bytes_sent) + " bytes instead of " + std::to_string(total_bytes) + "bytes");
}

void Session::RecvData(char* buffer, uint32_t total_bytes)
{
	uint32_t bytes_received = 0;

	while (bytes_received < total_bytes)
	{
		int32_t iResult = static_cast<int32_t>(recv(fd, buffer + bytes_received, total_bytes - bytes_received, 0));
				
		if (iResult < 0)
			throw std::runtime_error("Failed to receive data");

		// iResult here is always >= 0 meaning we can add it to an unsigned int
		bytes_received += iResult;
	}

	if (bytes_received != total_bytes)
		throw std::runtime_error("Received " + std::to_string(bytes_received) + " bytes instead of " + std::to_string(total_bytes) + "bytes");
}

// INT32
void Session::SendInt32(const int32_t integer)
{
	// Convert from Host Byte Order to Network Byte Order
	int32_t netInt = htonl(integer);
	
	SendData(reinterpret_cast<const char*>(&netInt), sizeof(int32_t));
}

void Session::RecvInt32(int32_t* integer)
{
	int32_t netInt = 0;
	RecvData(reinterpret_cast<char*>(&netInt), sizeof(int32_t));
	// Convert long from Network Byte Order to Host Byte Order
	*integer = ntohl(netInt);
}

// UINT32
void Session::SendUint32(const uint32_t integer)
{
	// Convert from Host Byte Order to Network Byte Order
	uint32_t netUint = htonl(integer);

	SendData(reinterpret_cast<const char*>(&netUint), sizeof(uint32_t));
}

void Session::RecvUint32(uint32_t* integer)
{
	uint32_t netUint = 0;
	RecvData(reinterpret_cast<char*>(&netUint), sizeof(uint32_t));
	// Convert long from Network Byte Order to Host Byte Order
	*integer = ntohl(netUint);
}

// BOOL
void Session::SendBoolean(const bool value)
{
	SendData(reinterpret_cast<const char*>(&value), sizeof(bool));
}

void Session::RecvBoolean(bool* value)
{
	RecvData(reinterpret_cast<char*>(&value), sizeof(bool));
}

// STD::STRING
void Session::SendString(const std::string& str)
{
	uint32_t buff_len = static_cast<uint32_t>(str.size());

	SendUint32(buff_len);
	SendData(str.c_str(), buff_len);
}

void Session::RecvString(std::string* str)
{
	uint32_t buff_len;

	RecvUint32(&buff_len);

	str->clear();
	str->resize(buff_len); // can throw std::bad_alloc if not enough memory
	
	// Receive the string
	RecvData(str->data(), buff_len);
}
