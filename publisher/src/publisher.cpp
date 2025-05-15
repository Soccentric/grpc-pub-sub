/**
 * @file publisher.cpp
 * @brief Implementation of the Publisher client for the PubSub gRPC service.
 */

#include "publisher.h"
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
                  << response.message_id() << std::endl;
        return true;
    } else {
        std::cerr << "Error publishing message: " << status.error_code() << ": " 
                  << status.error_message() << std::endl;
        return false;
    }
}
