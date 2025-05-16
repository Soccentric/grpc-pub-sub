/**
 * @file pubsub_service.cpp
 * @brief Implementation of the PubSub gRPC service for publishing and subscribing to messages.
 */
#include "pubsub_service.h"
#include "pubsub_common.h"
#include <thread>
#include <chrono>
#include <algorithm>

/**
 * @brief Constructor for PubSubServiceImpl
 * @param max_messages_per_topic Maximum number of messages to store per topic
 */
PubSubServiceImpl::PubSubServiceImpl(size_t max_messages_per_topic)
    : max_messages_per_topic_(max_messages_per_topic) {}

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
    
    // Store the message using our helper function
    AddMessageToTopic(topic, msg);
    
    std::cout << "Published message: " << content 
              << " to topic: " << topic 
              << " with ID: " << message_id 
              << " (Total messages in topic: " << GetMessageCount(topic) << ")" << std::endl;
    
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
    std::vector<std::string> topics;
    
    // First check if the topics repeated field is used
    if (request->topics_size() > 0) {
        for (int i = 0; i < request->topics_size(); i++) {
            topics.push_back(request->topics(i));
        }
    } 
    // Fallback to the legacy topic field with comma separation
    else {
        std::string topic_str = request->topic();
        size_t start = 0, end = 0;
        while ((end = topic_str.find(',', start)) != std::string::npos) {
            topics.push_back(topic_str.substr(start, end - start));
            start = end + 1;
        }
        topics.push_back(topic_str.substr(start));
    }
    
    SetFilterTopics(topics);

    std::cout << "New subscriber for " << topics.size() << " topics: ";
    for (const auto& t : filter_topics_) std::cout << t << " ";
    std::cout << std::endl;

    // Keep track of the last message ID processed for each topic
    std::unordered_map<std::string, std::string> last_msg_id_by_topic;
    
    while (!context->IsCancelled()) {
        std::vector<Message> messages_to_send;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& topic : filter_topics_) {
                auto it = messages_by_topic_.find(topic);
                if (it != messages_by_topic_.end()) {
                    for (const auto& msg : it->second) {
                        auto topic_it = last_msg_id_by_topic.find(topic);
                        if (topic_it == last_msg_id_by_topic.end() || 
                            msg.message_id() > topic_it->second) {
                            messages_to_send.push_back(msg);
                            last_msg_id_by_topic[topic] = msg.message_id();
                        }
                    }
                }
            }
        }
        
        // Sort messages by timestamp to ensure chronological delivery
        std::sort(messages_to_send.begin(), messages_to_send.end(),
                 [](const Message& a, const Message& b) {
                     return a.timestamp() < b.timestamp();
                 });
        
        for (const auto& msg : messages_to_send) {
            writer->Write(msg);
            std::cout << "Sent message: " << msg.content() 
                      << " (ID: " << msg.message_id() << ")"
                      << " to subscriber on topic: " << msg.topic() << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Subscriber disconnected from topics." << std::endl;
    return Status::OK;
}

void PubSubServiceImpl::SetFilterTopics(const std::vector<std::string>& topics) {
    filter_topics_ = topics;
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
 * @brief Add a message to a topic queue, managing the maximum size
 * @param topic The topic to add the message to
 * @param message The message to add
 */
void PubSubServiceImpl::AddMessageToTopic(const std::string& topic, const Message& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& topic_messages = messages_by_topic_[topic];
    topic_messages.push_back(message);
    
    // If we've exceeded the max messages per topic, remove the oldest messages
    if (topic_messages.size() > max_messages_per_topic_) {
        topic_messages.erase(topic_messages.begin(), 
                            topic_messages.begin() + (topic_messages.size() - max_messages_per_topic_));
    }
}

/**
 * @brief Get a list of all active topics
 * @return Vector of topic names
 */
std::vector<std::string> PubSubServiceImpl::GetAllTopics() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    std::vector<std::string> topics;
    topics.reserve(messages_by_topic_.size());
    
    for (const auto& pair : messages_by_topic_) {
        topics.push_back(pair.first);
    }
    
    return topics;
}

/**
 * @brief Get message count for a specific topic
 * @param topic The topic name
 * @return Number of messages stored for the topic
 */
size_t PubSubServiceImpl::GetMessageCount(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    auto it = messages_by_topic_.find(topic);
    if (it != messages_by_topic_.end()) {
        return it->second.size();
    }
    return 0;
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
 * @param max_messages_per_topic Maximum number of messages to store per topic (defaults to 100)
 */
void RunServer(const std::string& server_address, size_t max_messages_per_topic) {
    PubSubServiceImpl service(max_messages_per_topic);
    
    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with clients
    builder.RegisterService(&service);
    // Assemble the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    std::cout << "Maximum messages per topic: " << max_messages_per_topic << std::endl;
    std::cout << "Ready to handle publish/subscribe requests..." << std::endl;
    
    // Wait for the server to shutdown
    server->Wait();
}
