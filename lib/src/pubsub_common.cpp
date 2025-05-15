/**
 * @file pubsub_common.cpp
 * @brief Common utility functions for the PubSub system.
 */

#include "pubsub_common.h"
#include <string>
#include <chrono>

namespace pubsub {
namespace common {

/**
 * @brief Generate a unique message ID.
 * @return A unique string identifier for a message.
 */
std::string generateMessageId() {
    static int counter = 0;
    return std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) 
           + "-" + std::to_string(counter++);
}

/**
 * @brief Get the current timestamp.
 * @return The current time as an integer timestamp.
 */
int64_t getCurrentTimestamp() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

} // namespace common
} // namespace pubsub
