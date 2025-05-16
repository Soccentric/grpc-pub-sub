/**
 * @file publisher.cpp
 * @brief Implementation of the Publisher client for the PubSub gRPC service.
 */

#include "../include/publisher.h"
#include <iostream>
#include <grpcpp/grpcpp.h>
#include "pubsub.pb.h"
#include "pubsub.grpc.pb.h"
#include "pubsub_common.h"

using grpc::ClientContext;
using grpc::Status;
using pubsub::PublishRequest;
using pubsub::PublishResponse;

/**
 * @brief Constructs a Publisher client.
 * @param channel Shared pointer to the gRPC channel
 */
Publisher::Publisher(std::shared_ptr<Channel> channel)
    : stub_(PubSub::NewStub(channel)) {}

/**
 * @brief Publishes a message to a topic.
 *
 * Sends a message with the specified topic and content to the PubSub server.
 *
 * @param topic The topic to publish to
 * @param content The message content
 * @return true if the message was published successfully, false otherwise
 */
bool Publisher::Publish(const std::string& topic, const std::string& content) {
    // Create request
    PublishRequest request;
    request.set_topic(topic);
    request.set_content(content);

    // Set up context and response objects
    PublishResponse response;
    ClientContext context;

    // Call RPC
    Status status = stub_->Publish(&context, request, &response);

    if (status.ok()) {
        std::cout << "Message published successfully. Message ID: " 
                  << response.message_id() << " Topic: " << topic << std::endl;
        return true;
    } else {
        std::cerr << "Error publishing message: " << status.error_code() << ": " 
                  << status.error_message() << " Topic: " << topic << std::endl;
        return false;
    }
}

/**
 * @brief Publish a message to multiple topics.
 * @param topics Vector of topics to publish to
 * @param content The message content
 * @return Number of topics the message was successfully published to
 */
int Publisher::PublishToMultiple(const std::vector<std::string>& topics, const std::string& content) {
    int success_count = 0;
    for (const auto& topic : topics) {
        if (Publish(topic, content)) {
            success_count++;
        }
    }
    return success_count;
}

/**
 * @brief Register default topics for this publisher.
 * @param topics Vector of topics to register
 */
void Publisher::RegisterTopics(const std::vector<std::string>& topics) {
    registered_topics_ = topics;
    std::cout << "Registered " << topics.size() << " topics: ";
    for (const auto& topic : topics) {
        std::cout << topic << " ";
    }
    std::cout << std::endl;
}

/**
 * @brief Publish a message to all registered topics.
 * @param content The message content
 * @return Number of topics the message was successfully published to
 */
int Publisher::PublishToAll(const std::string& content) {
    if (registered_topics_.empty()) {
        std::cout << "No registered topics. Message not published." << std::endl;
        return 0;
    }
    return PublishToMultiple(registered_topics_, content);
}
