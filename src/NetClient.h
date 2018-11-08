#pragma once

#include <string>

class NetConnection;

class NetClient {
public:
	NetClient(std::string hostAddress, uint16_t port);
	~NetClient();

	bool isConnected() const;
	NetConnection& getConnection() const;

private:
};
