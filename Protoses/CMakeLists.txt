
project(Protoses)

FILE(GLOB ProtoMessagess protos/*.proto)

find_package(protobuf REQUIRED)
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION) 

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${ProtoMessagess})

include_directories(${PROTOBUF_INCLUDE_DIRS})
add_library(Protoses ${PROTO_SRCS} ${PROTO_HDRS})

protobuf_generate(TARGET Protoses LANGUAGE grpc 
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
  PROTOS ${ProtoMessagess}
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}" 
) 

target_link_libraries(Protoses protobuf::protobuf gRPC::grpc gRPC::grpc++ asio-grpc::asio-grpc)

