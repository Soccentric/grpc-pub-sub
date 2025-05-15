/**
 * @file main.cpp
 * @brief Entry point for the Publisher client application.
 *
 * This application connects to the PubSub gRPC server and publishes messages to a specified topic.
 * Command line arguments can be used to specify the server address, topic, and message content.
 */

#include "publisher.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <grpcpp/grpcpp.h>

/**
 * @brief Main function for the Publisher client.
 *
 * Connects to the PubSub server and publishes messages to a topic in a loop.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Exit status
 */
int main(int argc, char** argv) {
    // Default values
    std::string server_address = "localhost:50051";
    std::string topic = "default2opic";
    std::string content = "Hello from the publisher!";
    
    // Parse command line arguments
    if (argc > 1) server_address = argv[1];
    if (argc > 2) topic = argv[2];
    if (argc > 3) content = argv[3];
    
    // Create a channel to the server
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    Publisher publisher(channel);
    
    // Publish messages in a loop
    int counter = 0;
    while (true) {
        std::string message = content + " #" + std::to_string(counter++);
        std::cout << "Publishing: " << message << " to topic: " << topic << std::endl;
        
        publisher.Publish(topic, message);
        
        // Sleep for a second
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Optional: break after 10 messages
        if (counter >= 100) {
            break;
        }
    }

    return 0;
}
