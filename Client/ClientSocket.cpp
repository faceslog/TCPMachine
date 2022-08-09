#include "ClientSocket.hpp"

#include <ws2tcpip.h>
#include <stdexcept>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace TCPMachine;

ClientSocket::ClientSocket(std::string host, std::string port) : host(host), port(port)
{
	if(InitSocket() < 0)
		throw std::runtime_error("Failed to init socket !");
}

ClientSocket::~ClientSocket()
{
	if (connSocket != INVALID_SOCKET)
	{
		closesocket(connSocket);
		WSACleanup();
	}
}

int ClientSocket::InitSocket()
{
	WSADATA wsaData;
	struct addrinfo* result = nullptr, * ptr = nullptr, hints{};
	int iResult = -1;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("[ERROR] Getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	ZeroMemory(&hints, sizeof(hints));
	// AF_UNSPEC so the returned IP address could be either an IPv6 or IPv4 address for the server.
	// AF_INET6 for IPv6 or AF_INET for IPv4 in the hints parameter.
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0)
	{
		printf("[ERROR] Getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		connSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (connSocket == INVALID_SOCKET)
		{
			printf("[ERROR] Socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		// Connect to server.
		iResult = connect(connSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(connSocket);
			connSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (connSocket == INVALID_SOCKET)
	{
		printf("[INFO] Unable to connect to server!\n");
		WSACleanup();
		return -1;
	}

	return 0;
}

void ClientSocket::SendData(const char* buffer, uint32_t total_bytes)
{
	uint32_t bytes_sent = 0;

	while (bytes_sent < total_bytes)
	{
		int32_t iResult = static_cast<int32_t>(send(connSocket, buffer + bytes_sent, total_bytes - bytes_sent, 0));

		if (iResult < 0)
			throw std::runtime_error("Failed to send data");

		// iResult here is always >= 0 meaning we can add it to an unsigned int
		bytes_sent += iResult;
	}

	if (bytes_sent != total_bytes)
		throw std::runtime_error("Sent " + std::to_string(bytes_sent) + " bytes instead of " + std::to_string(total_bytes) + "bytes");
}

void ClientSocket::RecvData(char* buffer, uint32_t total_bytes)
{
	uint32_t bytes_received = 0;

	while (bytes_received < total_bytes)
	{
		int32_t iResult = static_cast<int32_t>(recv(connSocket, buffer + bytes_received, total_bytes - bytes_received, 0));

		if (iResult < 0)
			throw std::runtime_error("Failed to receive data");

		// iResult here is always >= 0 meaning we can add it to an unsigned int
		bytes_received += iResult;
	}

	if (bytes_received != total_bytes)
		throw std::runtime_error("Received " + std::to_string(bytes_received) + " bytes instead of " + std::to_string(total_bytes) + "bytes");
}

// INT32
void ClientSocket::SendInt32(const int32_t integer)
{
	// Convert from Host Byte Order to Network Byte Order
	int32_t netInt = htonl(integer);

	SendData(reinterpret_cast<const char*>(&netInt), sizeof(int32_t));
}

void ClientSocket::RecvInt32(int32_t* integer)
{
	int32_t netInt = 0;
	RecvData(reinterpret_cast<char*>(&netInt), sizeof(int32_t));
	// Convert long from Network Byte Order to Host Byte Order
	*integer = ntohl(netInt);
}

// UINT32
void ClientSocket::SendUint32(const uint32_t integer)
{
	// Convert from Host Byte Order to Network Byte Order
	uint32_t netUint = htonl(integer);

	SendData(reinterpret_cast<const char*>(&netUint), sizeof(uint32_t));
}

void ClientSocket::RecvUint32(uint32_t* integer)
{
	uint32_t netUint = 0;
	RecvData(reinterpret_cast<char*>(&netUint), sizeof(uint32_t));
	// Convert long from Network Byte Order to Host Byte Order
	*integer = ntohl(netUint);
}

// BOOL
void ClientSocket::SendBoolean(const bool value)
{
	SendData(reinterpret_cast<const char*>(&value), sizeof(bool));
}

void ClientSocket::RecvBoolean(bool* value)
{
	RecvData(reinterpret_cast<char*>(&value), sizeof(bool));
}

// STD::STRING
void ClientSocket::SendString(const std::string& str)
{
	uint32_t buff_len = static_cast<uint32_t>(str.size());

	SendUint32(buff_len);
	SendData(str.c_str(), buff_len);
}

void ClientSocket::RecvString(std::string* str)
{
	uint32_t buff_len;

	RecvUint32(&buff_len);

	str->clear();
	str->resize(buff_len); // can throw std::bad_alloc if not enough memory

	// Receive the string
	RecvData(str->data(), buff_len);
}