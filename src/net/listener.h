#pragma once

#include "result.h"
#include "connection.h"

#include <functional>

namespace net {

using listener = unsigned;

using newConnectionCallback = std::function<void(connection)>;

// attempts to open a listener on the specified port, for all interfaces.
// returns ok and fills outLis on success.
// returns error code on failure.
// the provided callback is called asynchronously each time a new client connects.
// the call returns as soon as the listener is established or an error is encountered;
// the listener will run on a separate internal thread
result startListen(uint16_t port, listener &outLis, newConnectionCallback callback);

// turns off a listener and closes the associated port
void stopListen(listener lis);

}
