/**
 * @file subscriber_client.h
 * @brief Declaration of the Subscriber client for the PubSub gRPC service.
 */
#ifndef SUBSCRIBER_CLIENT_H
#define SUBSCRIBER_CLIENT_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "pubsub.pb.h"
#include "pubsub.grpc.pb.h"

using grpc::Channel;
using pubsub::PubSub;
using pubsub::Message;

/**
 * @class SubscriberClient
 * @brief Client for consuming messages from the PubSub gRPC service.
 */
class SubscriberClient {
public:
    using MessageCallbackFn = std::function<void(const Message&)>;
    using TopicCallbackFn = std::function<void(const std::string&, const Message&)>;

    /**
     * @brief Constructs a Subscriber client.
     * @param channel Shared pointer to the gRPC channel
     */
    SubscriberClient(std::shared_ptr<Channel> channel);
    
    /**
     * @brief Destructor that ensures subscription thread is stopped.
     */
    ~SubscriberClient();
    
    /**
     * @brief Subscribe to a single topic with callback.
     * @param topic The topic to subscribe to
     * @param callback The function to call when a message is received
     * @return true if subscription was started successfully
     */
    bool Subscribe(const std::string& topic, MessageCallbackFn callback);
    
    /**
     * @brief Subscribe to multiple topics with a callback for each topic.
     * @param topics The topics to subscribe to
     * @param callback The function to call when a message is received, includes topic
     * @return true if subscription was started successfully
     */
    bool SubscribeToMultiple(const std::vector<std::string>& topics, TopicCallbackFn callback);
    
    /**
     * @brief Stop the subscription thread.
     */
    void Stop();

private:
    std::unique_ptr<PubSub::Stub> stub_;
    std::thread subscription_thread_;
    bool running_;
    
    void SubscriptionThread(const std::vector<std::string>& topics, TopicCallbackFn callback);
};

#endif // SUBSCRIBER_CLIENT_H
