#include "Server.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

using namespace TCPMachine;

Server::Server(uint16_t port, uint8_t nbWorkers) : sessions(nbWorkers)
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
	std::unique_lock<std::mutex> lock(guardStartStop);

	if (isRunning.load())
	{
		std::cerr << "[ERROR] [SERVER] : Server already started...\n" << std::endl;
		return -1;
	}
		
	if (not isRunning.is_lock_free())
		return -1;

	isRunning.store(true);
	
	handle = std::thread(&Server::ListenerThread, this);

	return 0;
}

int Server::Stop()
{
	std::unique_lock<std::mutex> lock(guardStartStop);

	if (not isRunning.load())
	{
		std::cerr << "[ERROR] [SERVER] : Server not started...\n" << std::endl;
		return -1;
	}
		
	isRunning.store(false);

	if (handle.joinable())
		handle.join();
	else
		return -1;

	return 0;
}

void Server::ListenerThread()
{
	int listen_sd = CreateListenSock();

	if (listen_sd < 0) 
		return;

	// ================== Init Threads Workers ==================
	if (sessions.StartWorkers() < 0)
	{
		std::cerr << "[ERROR] [SERVER] : Failed to start server (workers dead)...\n" << std::endl;
		close(listen_sd);
		return;
	}

	// ================== Wait for connections ==================
	while (isRunning.load())
	{
		int client_fd = accept(listen_sd, nullptr, nullptr);

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

		//  TO DO: Use poll() or select() to push only active sockets...
		sessions.Push(client_fd);
	}
	// ================== Stop Threads Workers ==================
	sessions.StopWorkers();

	// ================== Close the listener fd =================
	close(listen_sd);
	std::cout << "[SERVER] : Listener Thread Gracefully Stopped" << std::endl;
}

int Server::CreateListenSock()
{
	int serverfd = -1;
	int opt = 1;
	int flags = -1;

	struct sockaddr_in6 addr;

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	addr.sin6_addr = in6addr_any;

	// ================== Create Socket ==================
	if ((serverfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "[SERVER] : Socket Creation Failed" << std::endl;
		return -1;
	}
	std::cout << "[SERVER] : Socket Created Successfully" << std::endl;
	
	// ===== Allow socket descriptor to be reuseable  =====
	if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0)
	{
		std::cerr << "[SERVER] : Setsockopt() failed" << std::endl;
		close(serverfd);
		return -1;
	}
	std::cout << "[SERVER] : Socket Marked as Reuseable" << std::endl;

	// ================== Get Socket Flags ==================
	if ((flags = fcntl(serverfd, F_GETFL, 0)) < 0)
	{
		std::cerr << "[SERVER] : Could not get Flags on TCP Listening Socket" << std::endl;
		close(serverfd);
		return -1;
	}
	std::cout << "[SERVER] : Socket Flags Received" << std::endl;

	// ================== Set Socket Non Blocking ==================
	if (fcntl(serverfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		std::cerr << "[SERVER] : Could not set TCP Listening Socket to be Non-Blocking" << std::endl;
		close(serverfd);
		return -1;
	}
	std::cout << "[SERVER] : Socket Flags Sets" << std::endl;

	// ================== Bind Socket ==================
	if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		std::cerr << "[SERVER] : Bind failed" << std::endl;
		close(serverfd);
		return -1;
	}
	std::cout << "[SERVER] : Bind Successful." << std::endl;

	// ================== Set Listen Set ==================
	if (listen(serverfd, SOMAXCONN) < 0)
	{
		std::cerr << "[SERVER] : Listen Failed" << std::endl;
		close(serverfd);
		return -1;
	}
	std::cout << "[SERVER] : Socket in Listen State " << std::endl;

	return serverfd;
}

