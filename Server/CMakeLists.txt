
project (Server)

add_executable(Server Main.cpp )


target_link_libraries(Server ${Boost_LIBRARIES} protobuf::libprotobuf gRPC::grpc gRPC::grpc++ Protoses asio-grpc::asio-grpc)
