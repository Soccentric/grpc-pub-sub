/**
 * @file main.cpp
 * @brief Entry point for the Publisher client application.
 *
 * This application connects to the PubSub gRPC server and publishes messages to a specified topic.
 * Command line arguments can be used to specify the server address, topic, and message content.
 */

#include "../include/publisher.h"
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
    std::string topics_arg = "default_topic";
    std::string content = "Hello from the publisher!";
    
    // Parse command line arguments
    if (argc > 1) server_address = argv[1];
    if (argc > 2) topics_arg = argv[2];
    if (argc > 3) content = argv[3];
    
    // Parse comma-separated topics
    std::vector<std::string> topics;
    size_t start = 0, end = 0;
    while ((end = topics_arg.find(',', start)) != std::string::npos) {
        topics.push_back(topics_arg.substr(start, end - start));
        start = end + 1;
    }
    topics.push_back(topics_arg.substr(start));
    
    // Create a channel to the server
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    Publisher publisher(channel);
    
    // Register topics with the publisher
    publisher.RegisterTopics(topics);
    
    // Publish messages in a loop
    int counter = 0;
    bool use_all_topics = true;  // Set to false to alternate between topics
    
    while (true) {
        std::string message = content + " #" + std::to_string(counter++);
        
        if (use_all_topics) {
            // Publish to all registered topics
            std::cout << "Publishing: " << message << " to all registered topics" << std::endl;
            publisher.PublishToAll(message);
        } else {
            // Alternate between topics
            std::string topic = topics[counter % topics.size()];
            std::cout << "Publishing: " << message << " to topic: " << topic << std::endl;
            publisher.Publish(topic, message);
        }
        
        // Sleep for a second
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Optional: break after 100 messages
        if (counter >= 100) {
            break;
        }
    }

    return 0;
}
