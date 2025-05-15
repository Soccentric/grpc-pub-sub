/**
 * @file main.cpp
 * @brief Entry point for the Subscriber server application.
 *
 * This application starts the PubSub gRPC server and listens for incoming connections from clients.
 * The server address can be specified via command line arguments.
 */

#include "pubsub_service.h"

/**
 * @brief Main function for the Subscriber server.
 *
 * Starts the gRPC server to handle publish and subscribe requests.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Exit status
 */
int main(int argc, char** argv) {
    std::string server_address = "0.0.0.0:50051";
    
    // Parse command line arguments
    if (argc > 1) server_address = argv[1];
    
    RunServer(server_address);
    
    return 0;
}
