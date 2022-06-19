#include <iostream>
#include <signal.h>
#include <future>

#include "Server.hpp"

#define PORT 14005
#define WORKERS 2

#define DEBUG

int main()
{
    std::cout << "[TCPMACHINE] : Creating Signal Handler" << std::endl;

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);

#ifdef DEBUG
    std::cout << "[TCPMACHINE] : Adding SIGTRAP for Debugging" << std::endl;
    sigaddset(&sigset, SIGTRAP);
#endif // DEBUG
        
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    TCPMachine::Server srv(PORT, WORKERS);

    auto signal_handler = [&srv, &sigset]() 
    {
        int signum = 0;
        // wait until a signal is delivered:
        sigwait(&sigset, &signum);
       
        srv.Stop();
      
        return signum;
    };

    auto ft_signal_handler = std::async(std::launch::async, signal_handler);

    std::cout << "[TCPMACHINE] : Handler is Ready, Starting Server..." << std::endl;
    std::cout << "[TCPMACHINE] : Waiting for SIGTERM or SIGINT ([CTRL]+[c])" << std::endl;

    // 1 Thread -> Signal Handler | 1 Thread -> Listener | X Threads -> Worker = 2 + WORKERS
    std::cout << "[TCPMACHINE] : Using " << (WORKERS + 2) << " Threads" << std::endl;
    
    srv.Start();   

    int signal = ft_signal_handler.get();

    std::cout << "[TCPMACHINE] : Received signal: " << signal << std::endl;
    std::cout << "[TCPMACHINE] : Exiting Gracefully !" << std::endl;

    return EXIT_SUCCESS;
}