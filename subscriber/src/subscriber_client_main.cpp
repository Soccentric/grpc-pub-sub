/**
 * @file subscriber_client_main.cpp
 * @brief Main entry point for the subscriber client application.
 */

#include "subscriber_client.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <grpcpp/grpcpp.h>

// Signal handling for clean shutdown
std::atomic<bool> running(true);

void signalHandler(int signal) {
    std::cout << "Interrupt received, shutting down..." << std::endl;
    running.store(false);
}

/**
 * @brief Main function for the subscriber client.
 * 
 * This application connects to a PubSub server and subscribes to specified topics.
 * It handles messages from topics separately and maintains statistics for each topic.
 */
int main(int argc, char** argv) {
    // Set up signal handler
    std::signal(SIGINT, signalHandler);

    // Default values
    std::string server_address = "localhost:50051";
    std::string topics_arg = "default_topic";
    
    // Parse command line arguments
    if (argc > 1) server_address = argv[1];
    if (argc > 2) topics_arg = argv[2];
    
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
    
    // Create the subscriber client
    SubscriberClient subscriber(channel);
    
    // Keep track of message counts for each topic
    std::unordered_map<std::string, int> message_counts;
    
    // Subscribe to the topics
    subscriber.SubscribeToMultiple(topics, [&message_counts](const std::string& topic, const pubsub::Message& msg) {
        // Process the message
        std::cout << "Received message from topic '" << topic << "': " 
                  << msg.content() << " (ID: " << msg.message_id() << ")" << std::endl;
        
        // Increment the message count for this topic
        message_counts[topic]++;
    });
    
    std::cout << "Subscriber client started. Press Ctrl+C to stop." << std::endl;
    
    // Display statistics periodically
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        std::cout << "\n===== Message Statistics =====" << std::endl;
        for (auto const& pair : message_counts) {
            std::cout << "Topic '" << pair.first << "': " << pair.second << " messages" << std::endl;
        }
        std::cout << "============================\n" << std::endl;
    }
    
    // Clean up
    subscriber.Stop();
    std::cout << "Subscriber client stopped." << std::endl;
    
    return 0;
}
