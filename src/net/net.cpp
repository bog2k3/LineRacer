#include "connection.h"
#include "listener.h"

#include <asio.hpp>

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace net {

using asio::ip::tcp;

static asio::io_context theIoContext;
static std::vector<tcp::socket*> connections;
static std::vector<tcp::acceptor*> listeners;
static std::condition_variable workAvail;
static std::mutex asyncOpMutex;					// used to synchronize the changes to io_context run thread's state with operations that create or destroy
												// async objects such as connections and listeners

static void checkStart();	// checks if a worker thread is running and if not it starts one for all async operations
static void checkFinish();	// checks if all connections and listeners are closed and if so, shuts down the worker thread
static result translateError(const asio::error_code &err);

static tcp::socket* getSocket(connection con) {
	assert(con < connections.size() && connections[con] != nullptr);
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	return connections[con];
}

result startListenImpl(tcp::acceptor* acceptor, newConnectionCallback callback) {
	tcp::socket* clientSocket = new tcp::socket(theIoContext);
	acceptor->async_accept(*clientSocket, [acceptor, clientSocket, callback](const asio::error_code& error) {
		std::lock_guard<std::mutex> lk(asyncOpMutex);
		if (!error) {
			connections.push_back(clientSocket);
			callback(result::ok, connections.size() - 1);
		} else {
			delete clientSocket;
			callback(translateError(error), -1u);
		}
		// recurse to continue listening for new clients:
		if (acceptor->is_open())	// (only if the operation wasn't canceled meanwhile)
			startListenImpl(acceptor, callback);
		else
			delete acceptor;
	});
	checkStart();
	workAvail.notify_one();
	return result::ok;
}

result startListen(uint16_t port, listener &outLis, newConnectionCallback callback) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	tcp::acceptor* acceptor = new tcp::acceptor(theIoContext, tcp::endpoint(tcp::v4(), port));
	listeners.push_back(acceptor);
	return startListenImpl(acceptor, callback);
}

void stopListen(listener lis) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	assert(lis < listeners.size() && listeners[lis] != nullptr);
	listeners[lis]->close();
	listeners[lis] = nullptr;
	checkFinish();
}

result connect(std::string host, uint16_t port, connection& outCon) {
	tcp::resolver resolver(theIoContext);
    tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
    tcp::socket* newSocket = new tcp::socket(theIoContext);
	asio::error_code err;
    auto it = asio::connect(*newSocket, endpoints, err);
	if (!err) {
		std::lock_guard<std::mutex> lk(asyncOpMutex);
		connections.push_back(newSocket);
		outCon = connections.size() - 1;
		return result::ok;
	} else {
		outCon = -1;
		return translateError(err);
	}
}

void connect_async(std::string host, uint16_t port, newConnectionCallback callback) {
	tcp::resolver resolver(theIoContext);
    tcp::resolver::query query(host, std::to_string(port));
	resolver.async_resolve(query, [callback] (const asio::error_code &err, tcp::resolver::iterator endpointIter) {
		if (err) {
			callback(translateError(err), -1u);
		} else {
			tcp::socket* newSocket = new tcp::socket(theIoContext);
			asio::async_connect(*newSocket, endpointIter, [newSocket, callback] (const asio::error_code &err, tcp::resolver::iterator endpointIter) {
				if (!err) {
					std::lock_guard<std::mutex> lk(asyncOpMutex);
					connections.push_back(newSocket);
					callback(result::ok, connections.size()-1);
				} else {
					callback(translateError(err), -1u);
				}
			});
			checkStart();
			workAvail.notify_one();
		}
	});
	checkStart();
	workAvail.notify_one();
}

void closeConnection(connection con) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	assert(con < connections.size() && connections[con] != nullptr);
	connections[con]->close();
	delete connections[con];
	connections[con] = nullptr;
	checkFinish();
}

result write(connection con, const void* buffer, size_t count) {
	auto socket = getSocket(con);
	asio::error_code err;
	asio::write(*socket, asio::buffer(buffer, count), err);
	return translateError(err);
}

result read(connection con, void* buffer, size_t bufSize, size_t count) {
	assert(count <= bufSize);
	auto socket = getSocket(con);
	asio::error_code err;
	asio::read(*socket, asio::buffer(buffer, count), err);
	return translateError(err);
}

void cancelOperations(connection con) {
	auto socket = getSocket(con);
	socket->cancel();
}

static std::atomic<bool> isContextThreadRunning { false };
static std::atomic<bool> signalContextThreadExit { false };
std::thread contextThread;

static void ioContextThread() {
	std::mutex condMtx;
	while(!signalContextThreadExit.load(std::memory_order_acquire)) {
		std::unique_lock<std::mutex> condLk(condMtx);
		workAvail.wait(condLk);
		if (signalContextThreadExit.load(std::memory_order_acquire))
			break;

		// here we do the asio work
		theIoContext.run();
		theIoContext.restart();
	}
	isContextThreadRunning.store(false, std::memory_order_release);
}


static void checkStart() {
	// we are currently under asyncOpMutex lock by the caller
	if (!isContextThreadRunning.load(std::memory_order_acquire)) {
		if (!isContextThreadRunning.load(std::memory_order_acquire)) {
			isContextThreadRunning.store(true, std::memory_order_release);
			contextThread = std::thread(&ioContextThread);
		}
	}
}

static void checkFinish() {
	// we are currently under asyncOpMutex lock by the caller
	// count how many live connections/listeners we have:
	unsigned nObjects = 0;
	for (auto &c : connections)
		nObjects += c != nullptr ? 1 : 0;
	for (auto &c : listeners)
		nObjects += c != nullptr ? 1 : 0;
	// if no more objects, we stop the context thread:
	if (nObjects == 0)
	{
		signalContextThreadExit.store(true, std::memory_order_release);
		workAvail.notify_one();
	}
}

static result translateError(const asio::error_code &err) {
	if (!err)
		return result::ok;
	else
	 	return result::err_unknown;
}

} // namespace
