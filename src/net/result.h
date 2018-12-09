#pragma once

namespace net {

enum class result {
	ok,				// the operation succeeded
	err_timeout,	// the operation failed due to a timeout while waiting for remote response
	err_aborted,	// the operation failed due to the connection being forcefully closed
	err_refused,	// the connection attempt failed because the host refused to connect
	err_unknown		// the operation failed for an unknown cause
};

}
