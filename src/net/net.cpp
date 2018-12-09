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

std::vector<tcp::socket*> connections;
std::atomic<int> nLiveConnections {0};
//std::vector<tcp::acceptor*> listeners;
static std::condition_variable workAvail;

static void checkStart();	// checks if a worker thread is running and if not it starts one for all async operations
static void checkFinish();	// checks if all connections and listeners are closed and if so, shuts down the worker thread

result startListen(uint16_t port, listener &outLis, newConnectionCallback callback) {
	// ...
	workAvail.notify_one();
}

void stopListen(listener lis) {
	//...
	checkFinish();
}

result connect(std::string host, uint16_t port, connection& outCon) {
	// ...
	workAvail.notify_one();
}

void closeConnection(connection con) {
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

static asio::io_context theIoContext;
static std::atomic<bool> isContextThreadRunning { false };
static std::atomic<bool> signalContextThreadExit { false };
std::mutex contextThreadStateMtx;
std::thread contextThread;

static void ioContextThread() {
	//std::unique_lock<std::mutex> lk;
	std::lock_guard<std::mutex> stateLk(contextThreadStateMtx);
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
	if (!isContextThreadRunning.load(std::memory_order_acquire)) {
		std::lock_guard<std::mutex> lk(contextThreadStateMtx);	// the lock can only be acquired if the thread is not running
		if (!isContextThreadRunning.load(std::memory_order_acquire)) {
			isContextThreadRunning.store(true, std::memory_order_release);
			contextThread = std::thread(&ioContextThread);
		}
	}
}

static void checkFinish() {
	if (nLiveConnections.load /*&& listeners.empty*/)
	{
		signalContextThreadExit.store(true, std::memory_order_release);
		workAvail.notify_one();
	}
}

} // namespace
