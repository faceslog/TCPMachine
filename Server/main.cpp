#include <iostream>
#include <signal.h>
#include <future>

#include "Server.hpp"

int main()
{
    std::cout << "[TCPMACHINE] : Creating Signal Handler" << std::endl;

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    sigaddset(&sigset, SIGTRAP); // For debugging with visual remotely
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    TCPMachine::Server srv(14005);

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

    srv.Start();   
    
    int signal = ft_signal_handler.get();
    std::cout << "[TCPMACHINE] : Received signal: " << signal << std::endl;

    std::cout << "[TCPMACHINE] : Exiting Gracefully !" << std::endl;

    return EXIT_SUCCESS;
}