/**
 * @file subscriber_client.cpp
 * @brief Implementation of the Subscriber client for the PubSub gRPC service.
 */
#include "subscriber_client.h"
#include <iostream>
#include <grpcpp/grpcpp.h>
#include "pubsub.pb.h"
#include "pubsub.grpc.pb.h"

using grpc::ClientContext;
using grpc::Status;
using pubsub::SubscribeRequest;
using pubsub::Message;

/**
 * @brief Constructs a Subscriber client.
 * @param channel Shared pointer to the gRPC channel
 */
SubscriberClient::SubscriberClient(std::shared_ptr<Channel> channel)
    : stub_(PubSub::NewStub(channel)), running_(false) {}

/**
 * @brief Destructor that ensures subscription thread is stopped.
 */
SubscriberClient::~SubscriberClient() {
    Stop();
}

/**
 * @brief Subscribe to a single topic with callback.
 * @param topic The topic to subscribe to
 * @param callback The function to call when a message is received
 * @return true if subscription was started successfully
 */
bool SubscriberClient::Subscribe(const std::string& topic, MessageCallbackFn callback) {
    std::vector<std::string> topics = {topic};
    return SubscribeToMultiple(topics, [callback](const std::string&, const Message& msg) {
        callback(msg);
    });
}

/**
 * @brief Subscribe to multiple topics with a callback for each topic.
 * @param topics The topics to subscribe to
 * @param callback The function to call when a message is received, includes topic
 * @return true if subscription was started successfully
 */
bool SubscriberClient::SubscribeToMultiple(const std::vector<std::string>& topics, TopicCallbackFn callback) {
    // Stop any existing subscription thread
    Stop();
    
    running_ = true;
    subscription_thread_ = std::thread(&SubscriberClient::SubscriptionThread, this, topics, callback);
    return true;
}

/**
 * @brief Stop the subscription thread.
 */
void SubscriberClient::Stop() {
    if (running_) {
        running_ = false;
        if (subscription_thread_.joinable()) {
            subscription_thread_.join();
        }
    }
}

/**
 * @brief Thread function that handles the subscription.
 * @param topics The topics to subscribe to
 * @param callback The function to call when a message is received
 */
void SubscriberClient::SubscriptionThread(const std::vector<std::string>& topics, TopicCallbackFn callback) {
    ClientContext context;
    SubscribeRequest request;
    
    // Add topics to the request
    for (const auto& topic : topics) {
        request.add_topics(topic);
    }
    
    std::cout << "Subscribing to topics: ";
    for (const auto& topic : topics) {
        std::cout << topic << " ";
    }
    std::cout << std::endl;
    
    auto reader = stub_->Subscribe(&context, request);
    
    Message message;
    while (running_ && reader->Read(&message)) {
        callback(message.topic(), message);
    }
    
    Status status = reader->Finish();
    if (!status.ok() && running_) {
        std::cerr << "Subscription stream broken: " << status.error_code() 
                  << ": " << status.error_message() << std::endl;
    }
    
    std::cout << "Subscription thread terminated." << std::endl;
}
