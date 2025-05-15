# gRPC Publish-Subscribe Example

A simple publish-subscribe system implemented using gRPC and Protocol Buffers in C++. This example demonstrates how to create a simple client-server application where:

- **Publisher**: Publishes messages to specific topics
- **Subscriber**: Subscribes to topics and receives a stream of messages

## Project Structure

```
.
├── CMakeLists.txt          # CMake build configuration
├── install.sh              # Script to install dependencies from packages
├── install_from_source.sh  # Script to install dependencies from source
├── lib/                    # Common shared library code
│   ├── include/
│   │   └── pubsub_common.h
│   └── src/
│       └── pubsub_common.cpp
├── proto/
│   └── pubsub.proto        # Protocol buffer service definition
├── publisher/              # Publisher application
│   ├── include/
│   │   └── publisher.h
│   └── src/
│       ├── main.cpp
│       └── publisher.cpp
└── subscriber/             # Subscriber server application
    ├── include/
    │   └── pubsub_service.h
    └── src/
        ├── main.cpp
        └── pubsub_service.cpp
```

## Prerequisites

- C++ compiler with C++14 support
- CMake (version 3.5 or higher)
- gRPC and Protocol Buffers libraries

## Installation

### Option 1: Install Dependencies from Packages

```bash
chmod +x install.sh
./install.sh
```

### Option 2: Install Dependencies from Source

If your system doesn't have gRPC packages or you need a specific version:

```bash
chmod +x install_from_source.sh
./install_from_source.sh
```

## Building the Project Manually

If you already have gRPC and Protocol Buffers installed:

```bash
mkdir -p build
cd build
cmake ..
make
```

## Running the Example

1. First, start the subscriber (server):

```bash
./build/subscriber [server_address]
```

By default, the server runs on `0.0.0.0:50051`.

2. In a different terminal, start the publisher (client):

```bash
./build/publisher [server_address] [topic] [message]
```

By default:
- Server address: `localhost:50051`
- Topic: `default`
- Message: `Hello from the publisher!`

## How It Works

1. The subscriber starts a gRPC server that implements the `PubSub` service.
2. The publisher connects to the server and publishes messages to a topic.
3. The server keeps track of messages per topic.
4. Subscribers receive all new messages published to their subscribed topic in real-time.

## Protocol Definition

The service is defined in `proto/pubsub.proto`:

- `Publish` RPC: Lets publishers send messages to a topic
- `Subscribe` RPC: Creates a server-streaming connection to deliver messages to subscribers

## Extending the Example

This example can be extended in several ways:
- Add authentication
- Implement message persistence
- Add support for wildcards in topic names
- Implement message filtering
- Add quality of service (QoS) options