/**
 * @file pubsub_service.h
 * @brief Declaration of the PubSub gRPC service implementation.
 */
#ifndef PUBSUB_SERVICE_H
#define PUBSUB_SERVICE_H

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include "pubsub.pb.h"
#include "pubsub.grpc.pb.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using pubsub::PubSub;
using pubsub::PublishRequest;
using pubsub::PublishResponse;
using pubsub::SubscribeRequest;
using pubsub::Message;

/**
 * @class PubSubServiceImpl
 * @brief Implementation of the PubSub gRPC service for handling publish and subscribe requests.
 */
class PubSubServiceImpl final : public PubSub::Service {
public:
    /**
     * @brief Constructor for PubSubServiceImpl
     * @param max_messages_per_topic Maximum number of messages to store per topic (defaults to 100)
     */
    PubSubServiceImpl(size_t max_messages_per_topic = 100);

    /**
     * @brief Publish a message to a topic.
     * @param context The gRPC server context
     * @param request The publish request
     * @param response The response to be sent back to the client
     * @return Status::OK if successful
     */
    Status Publish(ServerContext* context, const PublishRequest* request,
                  PublishResponse* response) override;
    
    /**
     * @brief Subscribe to a topic or topics and receive messages.
     * @param context The gRPC server context
     * @param request The subscribe request
     * @param writer The server writer used to stream messages to the client
     * @return Status::OK when the client disconnects
     */
    Status Subscribe(ServerContext* context, const SubscribeRequest* request,
                    ServerWriter<Message>* writer) override;

    /**
     * @brief Set the list of topics to filter for this subscriber.
     * @param topics The list of topics to subscribe to
     */
    void SetFilterTopics(const std::vector<std::string>& topics);
    
    /**
     * @brief Get a list of all active topics
     * @return Vector of topic names
     */
    std::vector<std::string> GetAllTopics() const;
    
    /**
     * @brief Get message count for a specific topic
     * @param topic The topic name
     * @return Number of messages stored for the topic
     */
    size_t GetMessageCount(const std::string& topic) const;

private:
    // Generate a unique message ID
    std::string GenerateMessageId();
    
    // Add a message to a topic queue, managing the maximum size
    void AddMessageToTopic(const std::string& topic, const Message& message);
    
    // Maximum number of messages to store per topic
    size_t max_messages_per_topic_;
    
    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<Message>> messages_by_topic_;
    std::vector<std::string> filter_topics_; // List of topics to filter for this subscriber
};

// Server runner function
void RunServer(const std::string& server_address, size_t max_messages_per_topic = 100);

#endif // PUBSUB_SERVICE_H
