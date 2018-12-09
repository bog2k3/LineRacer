#include "connection.h"
#include "listener.h"

#include <vector>
#include <mutex>

namespace net {

result startListen(uint16_t port, listener &outLis, newConnectionCallback callback) {

}

void stopListen(listener lis) {

}

result connect(std::string host, uint16_t port, connection& outCon) {

}

void closeConnection(connection con) {

}

result write(connection con, const void* buffer, size_t count) {

}

result read(connection con, void* buffer, size_t bufSize, size_t count) {


}

} // namespace
