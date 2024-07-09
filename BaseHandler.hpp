#include <grpc++/grpc++.h>

class BaseHandler {
public:
    virtual int Handle() = 0;
};

struct UserData;

using HandlerFunction = void(*)(BaseHandler*, UserData*); 

struct UserData{
    HandlerFunction pHandler = nullptr;
    void* pUserData = nullptr;
    BaseHandler* pThis = nullptr;
    ::grpc::Status status;
};
