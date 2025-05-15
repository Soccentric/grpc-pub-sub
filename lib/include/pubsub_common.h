/**
 * @file pubsub_common.h
 * @brief Common utility function declarations for the PubSub system.
 */

#ifndef PUBSUB_COMMON_H
#define PUBSUB_COMMON_H

#include <string>
#include <chrono>

namespace pubsub {
namespace common {

/**
 * @brief Generate a unique message ID.
 * @return A unique string identifier for a message.
 */
std::string generateMessageId();

/**
 * @brief Get the current timestamp.
 * @return The current time as an integer timestamp.
 */
int64_t getCurrentTimestamp();

} // namespace common
} // namespace pubsub

#endif // PUBSUB_COMMON_H
