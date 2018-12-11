#include "listener.h"
#include "connection.h"

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <cstring>
#include <iostream>

void testHost(int port);
void testClient(char* host, int port);

void testNet(int argc, char** argv) {
	if (argc < 3)
		throw std::runtime_error("wrong args");
	if (!strcmp(argv[1], "host"))
		testHost(atoi(argv[2]));
	else if (!strcmp(argv[1], "join")) {
		if (argc != 4)
			throw std::runtime_error("wrong args");
		else
			testClient(argv[2], atoi(argv[3]));
	} else
		throw std::runtime_error("wrong args");
}

std::string errName(net::result err) {
	switch (err) {
		case net::result::ok: return "OK";
		case net::result::err_timeout: return "timeout";
		case net::result::err_aborted: return "aborted";
		case net::result::err_refused: return "refused";
		case net::result::err_unknown: return "unknown";
		default: throw std::runtime_error("value not handled");
	}
}

void runChat(net::connection con, bool first) {
	bool myTurn = first;
	while (!std::cin.eof()) {
		if (myTurn) {
			std::string line;
			std::cin >> line;
			size_t len = line.size();
			auto err = net::write(con, &len, sizeof(len));
			if (err == net::result::ok)
				err = net::write(con, line.c_str(), len+1);
			if (err != net::result::ok)
				std::cout << "FAILED to send message: " << errName(err) << "\n";
		} else {
			char buf[1024];
			size_t len;
			auto err = net::read(con, &len, sizeof(len), sizeof(len));
			if (err == net::result::ok) {
				size_t recv = 0;
				while (recv < len) {
					size_t chunkSize = std::min(sizeof(buf)-1, len-recv);
					err = net::read(con, buf, sizeof(buf), chunkSize);
					if (err != net::result::ok) {
						std::cout << "FAILED to receive message: " << errName(err) << "\n";
					} else {
						buf[chunkSize+1] = 0;
						std::cout << buf;
						recv += chunkSize;
					}
				}
				std::cout << "\n";
			} else {
				std::cout << "FAILED to receive message: " << errName(err) << "\n";
			}
		}
		myTurn = !myTurn;
	}
}

void testHost(int port) {

}

void testClient(char* host, int port) {
	std::cout << "connecting to " << host << ":" << port << " ...\n";
	net::connection con;
	auto res = net::connect(host, port, con);
	if (res != net::result::ok) {
		std::cout << "FAILED to connect: " << errName(res) << "\n";
		return;
	} else {
		std::cout << "CONNECTED. Say hello!\n";
		runChat(con, true);
		std::cout << "closing connection...\n";
		net::closeConnection(con);
		std::cout << "Connection closed. EXITING.\n";
	}
}
