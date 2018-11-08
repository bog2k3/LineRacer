#pragma once

#include <asio.hpp>

#include <string>

class NetConnection;

class NetHost {
public:
	NetHost(std::string ip, uint16_t port, unsigned maxClients=8);
	~NetHost();

	bool isOpen() const;

	// returns a valid NetConnection if one is pending or nullptr if none available;
	// this will remove the NetConnection from the internal queue when returned, so make sure to save it
	NetConnection* getPendingClient() const;
private:
};
