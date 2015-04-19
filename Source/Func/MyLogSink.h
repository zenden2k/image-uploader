#include "Core/Logging.h"

class GOOGLE_GLOG_DLL_DECL MyLogSink: public google::LogSink {
public:
	virtual void send(google::LogSeverity severity, const char* full_filename,
		const char* base_filename, int line,
		const struct ::tm* tm_time,
		const char* message, size_t message_len);
}; 