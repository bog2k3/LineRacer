#pragma once

#include <string>

class NetConnection {
public:
	std::string endpointName() const;
	std::string userName() const;

	bool isAlive() const;

	void setTimeout(unsigned timeoutSec);

	// receive data into buffer; outRcvSize will contain the number of bytes received
	// returns true on success, false on timeout or connection closed
	bool receive(void* buf, size_t bufSize, size_t &outRcvSize);

	// send <size> bytes from buffer;
	// returns true on success, false on timeout or connection closed
	bool send(void* buf, size_t size);

private:
	friend class NetClient;
	friend class NetHost;

	NetConnection();
};
