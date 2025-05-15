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
     * @brief Publish a message to a topic.
     * @param context The gRPC server context
     * @param request The publish request
     * @param response The response to be sent back to the client
     * @return Status::OK if successful
     */
    Status Publish(ServerContext* context, const PublishRequest* request,
                  PublishResponse* response) override;
    
    /**
     * @brief Subscribe to a topic and receive messages.
     * @param context The gRPC server context
     * @param request The subscribe request
     * @param writer The server writer used to stream messages to the client
     * @return Status::OK when the client disconnects
     */
    Status Subscribe(ServerContext* context, const SubscribeRequest* request,
                    ServerWriter<Message>* writer) override;

private:
    // Generate a unique message ID
    std::string GenerateMessageId();
    
    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<Message>> messages_by_topic_;
};

// Server runner function
void RunServer(const std::string& server_address);

#endif // PUBSUB_SERVICE_H
