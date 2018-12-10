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

result startListen(uint16_t port, listener &outLis, newConnectionCallback callback) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	tcp::acceptor* acceptor = new tcp::acceptor(theIoContext, tcp::endpoint(tcp::v4(), port));
	listeners.push_back(acceptor);
	tcp::socket* clientSocket = new tcp::socket(theIoContext);
	acceptor->async_accept(*clientSocket, [clientSocket, &callback](const asio::error_code& error) {
		if (!error) {
			std::lock_guard<std::mutex> lk(asyncOpMutex);
			connections.push_back(clientSocket);
			callback(result::ok, connections.size() - 1);
		} else {
			delete clientSocket;
			callback(translateError(error), -1u);
		}
		#warning "this will stop after the first connection is accepted, must recall async_accept with a new socket"
	});
	checkStart();
	workAvail.notify_one();
	return result::ok;
}

void stopListen(listener lis) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	assert(lis < listeners.size() && listeners[lis] != nullptr);
	listeners[lis]->close();
	delete listeners[lis];
	listeners[lis] = nullptr;
	checkFinish();
}

result connect(std::string host, uint16_t port, connection& outCon) {
	tcp::resolver resolver(theIoContext);
    tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
    tcp::socket* newSocket = new tcp::socket(theIoContext);
    asio::error_code err = asio::connect(*socket, endpoints);
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
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	// ...
	checkStart();
	workAvail.notify_one();
}

void closeConnection(connection con) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	//...
	checkFinish();
}

result write(connection con, const void* buffer, size_t count) {
	// ...
	workAvail.notify_one();
}

result read(connection con, void* buffer, size_t bufSize, size_t count) {
	// ...
	workAvail.notify_one();
}

static std::atomic<bool> isContextThreadRunning { false };
static std::atomic<bool> signalContextThreadExit { false };
std::thread contextThread;

static void ioContextThread() {
	//std::unique_lock<std::mutex> lk;
	// std::lock_guard<std::mutex> stateLk(contextThreadStateMtx);
	// the lock is acquired as soon as checkStart finishes its initialization

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
