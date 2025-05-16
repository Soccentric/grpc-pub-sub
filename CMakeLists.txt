cmake_minimum_required(VERSION 3.5)
project(grpc_pubsub CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Get gRPC libraries
set(_GRPC_GRPCPP gRPC::grpc++)

# Set the path to grpc_cpp_plugin manually if it's not found
set(GRPC_CPP_PLUGIN_EXECUTABLE /usr/bin/grpc_cpp_plugin)

# Generate protobuf and gRPC code
get_filename_component(pb_proto_schema "./proto/pubsub.proto" ABSOLUTE)
get_filename_component(pb_proto_path "${pb_proto_schema}" PATH)

# Generate sources for proto file
set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/pubsub.pb.cc")
set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/pubsub.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/pubsub.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/pubsub.grpc.pb.h")

add_custom_command(
    OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${pb_proto_path}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${pb_proto_schema}"
    DEPENDS "${pb_proto_schema}")

# Set include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${CMAKE_SOURCE_DIR}/publisher/include)
include_directories(${CMAKE_SOURCE_DIR}/publisher1/include)
include_directories(${CMAKE_SOURCE_DIR}/publisher2/include)
include_directories(${CMAKE_SOURCE_DIR}/subscriber/include)

# Proto files
set(PROTO_FILES 
    ${proto_srcs}
    ${proto_hdrs}
    ${grpc_srcs}
    ${grpc_hdrs})

# Common library
add_library(pubsub_common
    lib/src/pubsub_common.cpp)

# Publisher library
add_library(publisher_lib
    publisher/src/publisher.cpp
    ${PROTO_FILES})
target_link_libraries(publisher_lib
    pubsub_common
    ${_GRPC_GRPCPP}
    ${PROTOBUF_LIBRARIES})

# Publisher executable
add_executable(publisher
    publisher/src/main.cpp
    ${proto_srcs}
    ${grpc_srcs})
target_link_libraries(publisher
    publisher_lib
    ${CMAKE_THREAD_LIBS_INIT})
    
# Publisher1 library
add_library(publisher1_lib
    publisher1/src/publisher.cpp
    ${PROTO_FILES})
target_include_directories(publisher1_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/publisher1/include)
target_link_libraries(publisher1_lib
    pubsub_common
    ${_GRPC_GRPCPP}
    ${PROTOBUF_LIBRARIES})

# Publisher1 executable
add_executable(publisher1
    publisher1/src/main.cpp
    ${proto_srcs}
    ${grpc_srcs})
target_include_directories(publisher1 PUBLIC
    ${CMAKE_SOURCE_DIR}/publisher1/include)
target_link_libraries(publisher1
    publisher1_lib
    ${CMAKE_THREAD_LIBS_INIT})

# Publisher2 library
add_library(publisher2_lib
    publisher2/src/publisher.cpp
    ${PROTO_FILES})
target_include_directories(publisher2_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/publisher2/include)
target_link_libraries(publisher2_lib
    pubsub_common
    ${_GRPC_GRPCPP}
    ${PROTOBUF_LIBRARIES})

# Publisher2 executable
add_executable(publisher2
    publisher2/src/main.cpp
    ${proto_srcs}
    ${grpc_srcs})
target_include_directories(publisher2 PUBLIC
    ${CMAKE_SOURCE_DIR}/publisher2/include)
target_link_libraries(publisher2
    publisher2_lib
    ${CMAKE_THREAD_LIBS_INIT})

# Subscriber service library
add_library(pubsub_service
    subscriber/src/pubsub_service.cpp
    ${PROTO_FILES})
target_link_libraries(pubsub_service
    pubsub_common
    ${_GRPC_GRPCPP}
    ${PROTOBUF_LIBRARIES})

# Subscriber client library
add_library(subscriber_client
    subscriber/src/subscriber_client.cpp
    ${PROTO_FILES})
target_link_libraries(subscriber_client
    pubsub_common
    ${_GRPC_GRPCPP}
    ${PROTOBUF_LIBRARIES})

# Subscriber server executable
add_executable(subscriber
    subscriber/src/main.cpp
    ${proto_srcs}
    ${grpc_srcs})
target_link_libraries(subscriber
    pubsub_service
    ${CMAKE_THREAD_LIBS_INIT})

# Subscriber client executable
add_executable(subscriber_client_app
    subscriber/src/subscriber_client_main.cpp
    ${proto_srcs}
    ${grpc_srcs})
target_compile_features(subscriber_client_app PRIVATE cxx_std_14)
target_link_libraries(subscriber_client_app
    subscriber_client
    ${CMAKE_THREAD_LIBS_INIT})