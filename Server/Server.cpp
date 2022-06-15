#include "Server.hpp"
#include "Session.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

using namespace TCPMachine;

Server::Server(uint16_t port) : sessions()
{
	this->isRunning.store(false);
	this->port = port;
}

Server::~Server() 
{
	if (isRunning.load())
	{
		Stop();
	}	
}

int Server::Start()
{
	if (isRunning.load())
		return -1;
	
	if (not isRunning.is_lock_free())
		return -1;

	isRunning.store(true);
	handler = std::thread(&Server::HandlerThread, this);

	return 0;
}

int Server::Stop()
{
	if (not isRunning.load())
		return -1;

	isRunning.store(false);

	if (handler.joinable())
		handler.join();
	else
		return -1;

	return 0;
}

// Created with the help of :
// https://jameshfisher.com/2017/04/05/set_socket_nonblocking/
// If our server only makes calls which select() has indicated will not block, will everything be OK? No!
// These two operations - select followed by the hopefully non-blocking call - are non-atomic.
void Server::HandlerThread() 
{
	struct sockaddr_in addr;
		
	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port			= htons(port);

	int addr_len			= sizeof(addr);
	int general_fd			= -1;
	int flags				= -1;

	// ================== Create Socket ==================
	if ((general_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "[SERVER] : Socket Creation Failed" << std::endl;
		return;
	}
	std::cout << "[SERVER] : Socket Created Successfully" << std::endl;
	
	// ================== Get Socket Flags ==================
	if ((flags = fcntl(general_fd, F_GETFL, 0)) < 0)
	{
		std::cerr << "[SERVER] : Could not get Flags on TCP Listening Socket" << std::endl;
		return;
	}
	std::cout << "[SERVER] : Socket Flags Received" << std::endl;

	// ================== Set Socket Non Blocking ==================
	if (fcntl(general_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		std::cerr << "[SERVER] : Could not set TCP Listening Socket to be Non-Blocking" << std::endl;
		return;
	}
	std::cout << "[SERVER] : Socket Flags Sets" << std::endl;

	// ================== Bind Socket ==================
	if (bind(general_fd, (struct sockaddr*)&addr, addr_len) < 0)
	{
		std::cerr << "[SERVER] : Bind failed" << std::endl;
		return;
	}
	std::cout << "[SERVER] : Bind Successful." << std::endl;

	// ================== Set Listen Set ==================
	if (listen(general_fd, SOMAXCONN) < 0)
	{
		std::cerr << "[SERVER] : Listen Failed" << std::endl;
		return;
	}
	std::cout << "[SERVER] : Socket in Listen State " << std::endl;
	
	// ================== Wait for connections ==================
	while (isRunning.load())
	{
		int client_fd = accept(general_fd, (struct sockaddr*)&addr, (socklen_t*)&addr_len);

		if (client_fd < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			else
			{
				std::cerr << "[ERROR] [SERVER] : Error when accepting connection\n" << std::endl;
				break;
			}
		}

		sessions.Add(client_fd);			
	}

	// ================== Cleanup the Server ==================
	sessions.TerminateAll();
	close(general_fd);

	std::cout << "[SERVER] : Handler Thread Gracefully Stopped" << std::endl;
}

