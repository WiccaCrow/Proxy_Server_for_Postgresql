#include <iostream>

#include "../include/Server.hpp"

Server server;

int main()
{
    server.start();

    std::cout << "Hello" << std::endl;
    
    return 0;
}
