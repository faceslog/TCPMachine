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
