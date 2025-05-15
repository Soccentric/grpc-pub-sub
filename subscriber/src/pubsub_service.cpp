/**
 * @file pubsub_service.cpp
 * @brief Implementation of the PubSub gRPC service for publishing and subscribing to messages.
 */
#include "pubsub_service.h"
#include "pubsub_common.h"
#include <thread>
#include <chrono>

/**
 * @brief Publishes a message to a specified topic
 * 
 * This method handles client requests to publish messages to a topic.
 * It generates a unique ID for each message, stores it in the server's
 * memory, and makes it available for subscribers.
 * 
 * @param context The gRPC server context
 * @param request The publish request containing topic and content
 * @param response The response to be sent back to the client
 * @return Status::OK if successful
 */
Status PubSubServiceImpl::Publish(ServerContext* context, const PublishRequest* request,
              PublishResponse* response) {
    std::string topic = request->topic();
    std::string content = request->content();
    
    // Generate a unique message ID
    std::string message_id = pubsub::common::generateMessageId();
    
    // Create a new message
    Message msg;
    msg.set_message_id(message_id);
    msg.set_topic(topic);
    msg.set_content(content);
    msg.set_timestamp(pubsub::common::getCurrentTimestamp());
    
    // Store the message
    std::lock_guard<std::mutex> lock(mutex_);
    messages_by_topic_[topic].push_back(msg);
    
    std::cout << "Published message: " << content 
              << " to topic: " << topic 
              << " with ID: " << message_id << std::endl;
    
    // Set the response
    response->set_success(true);
    response->set_message_id(message_id);
    
    return Status::OK;
}

/**
 * @brief Subscribes to a topic and streams messages to the client
 *
 * This method implements a streaming RPC that sends messages to subscribers.
 * It maintains a connection with the client until they disconnect, continuously
 * checking for and sending new messages that match the requested topic.
 * Each subscriber receives all messages published to the topic after they connect.
 *
 * @param context The gRPC server context
 * @param request The subscribe request containing the topic to subscribe to
 * @param writer The server writer used to stream messages to the client
 * @return Status::OK when the client disconnects
 */
Status PubSubServiceImpl::Subscribe(ServerContext* context, const SubscribeRequest* request,
                ServerWriter<Message>* writer) {
    std::string topic = request->topic();
    std::cout << "New subscriber for topic: " << topic << std::endl;
    
    // Keep track of the last message ID sent to this subscriber
    std::string last_msg_id;
    
    // Continue until the client disconnects
    while (!context->IsCancelled()) {
        // Check for new messages on this topic
        std::vector<Message> messages_to_send;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = messages_by_topic_.find(topic);
            if (it != messages_by_topic_.end()) {
                for (const auto& msg : it->second) {
                    if (last_msg_id.empty() || msg.message_id() > last_msg_id) {
                        messages_to_send.push_back(msg);
                        last_msg_id = msg.message_id();
                    }
                }
            }
        }
        
        // Send any new messages
        for (const auto& msg : messages_to_send) {
            writer->Write(msg);
            std::cout << "Sent message: " << msg.content() 
                      << " to subscriber on topic: " << topic << std::endl;
        }
        
        // Sleep a bit before checking for more messages
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Subscriber disconnected from topic: " << topic << std::endl;
    return Status::OK;
}

/**
 * @brief Generates a unique identifier for messages
 * 
 * This method creates a unique message ID by delegating to the
 * common utility function. This ensures consistency in ID generation
 * across the application.
 *
 * @return A string containing the unique message ID
 */
std::string PubSubServiceImpl::GenerateMessageId() {
    return pubsub::common::generateMessageId();
}

/**
 * @brief Runs the gRPC server with the PubSub service
 *
 * Initializes and starts a gRPC server on the specified address.
 * The server hosts the PubSubService implementation and runs until
 * explicitly shut down.
 *
 * @param server_address The address and port on which the server should listen
 *                       in the format "address:port" (e.g., "localhost:50051")
 */
void RunServer(const std::string& server_address) {
    PubSubServiceImpl service;
    
    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with clients
    builder.RegisterService(&service);
    // Assemble the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    
    // Wait for the server to shutdown
    server->Wait();
}
