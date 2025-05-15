/**
 * @file publisher.h
 * @brief Declaration of the Publisher client for the PubSub gRPC service.
 */
#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "pubsub.pb.h"
#include "pubsub.grpc.pb.h"

using grpc::Channel;
using pubsub::PubSub;

/**
 * @class Publisher
 * @brief Client for publishing messages to the PubSub gRPC service.
 */
class Publisher {
public:
    /**
     * @brief Constructs a Publisher client.
     * @param channel Shared pointer to the gRPC channel
     */
    Publisher(std::shared_ptr<Channel> channel);
    
    /**
     * @brief Publish a message to a topic.
     * @param topic The topic to publish to
     * @param content The message content
     * @return true if the message was published successfully, false otherwise
     */
    bool Publish(const std::string& topic, const std::string& content);

private:
    std::unique_ptr<PubSub::Stub> stub_;
};

#endif // PUBLISHER_H
