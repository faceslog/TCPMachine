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
    
    // Block signals in this thread and subsequently spawned threads
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    sigaddset(&sigset, SIGTRAP); // VS debugger uses SIGTRAP for remote dev
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    TCPMachine::Server srv(PORT, WORKERS);

    auto signal_handler = [&srv, &sigset]() 
    {
        int signum = 0;
        // wait until a signal is delivered:
        sigwait(&sigset, &signum);
       
        // Stop the server when the signal is delivred
        srv.Stop();
      
        return signum;
    };

    auto ft_signal_handler = std::async(std::launch::async, signal_handler);
    
    // Main + Server Listener + SigHandler + X Worker = WORKERS + 3
    std::cout << "[TCPMACHINE] : Using a total of: " << (WORKERS + 3) << " Threads" << std::endl;  
    std::cout << "[TCPMACHINE] : Handler is Ready, Starting Server..." << std::endl;
    std::cout << "[TCPMACHINE] : Waiting for SIGTERM or SIGINT ([CTRL]+[c])" << std::endl;
    
    srv.Start();   

    int signal = ft_signal_handler.get();

    std::cout << "[TCPMACHINE] : Received signal: " << signal << std::endl;
    std::cout << "[TCPMACHINE] : Exiting Gracefully !" << std::endl;

    return EXIT_SUCCESS;
}