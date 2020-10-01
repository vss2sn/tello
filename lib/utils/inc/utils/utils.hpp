#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <mutex>
#include <sstream>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

namespace utils_log{

class DisplayMutex {
public:
	static std::mutex& getMutex() {
		return display_mutex;
	}
private:
	static std::mutex display_mutex;
};

#define LogDebug() LogDebugDetailed(__FILENAME__, __LINE__)
#define LogInfo() LogInfoDetailed(__FILENAME__, __LINE__)
#define LogStatus() LogStatusDetailed(__FILENAME__, __LINE__)
#define LogWarn() LogWarnDetailed(__FILENAME__, __LINE__)
#define LogErr() LogErrorDetailed(__FILENAME__, __LINE__)

enum class Colour { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, RESET };
enum  LogLevel {Debug, Info, Warn, Err, Status};
void set_display_colour(Colour colour);

class LogDetailed{
public:
	LogDetailed(const char *filename, int filenumber):_s(), _caller_filename(filename), _caller_filenumber(filenumber) {}

	template <typename T> LogDetailed &operator<<(const T &x){
		_s << x;
		std::string _str = _s.str();
		return *this;
	}

	static void setLogLevel(LogLevel level);

	virtual ~LogDetailed();

protected:
	LogLevel _log_level;

private:
	std::stringstream _s;
	const char *_caller_filename;
	int _caller_filenumber;
	static LogLevel _min_level;
};

class LogDebugDetailed : public LogDetailed{
public:
	LogDebugDetailed(const char *filename, int filenumber) : LogDetailed(filename, filenumber){
		_log_level = LogLevel::Debug;
	}
};

class LogInfoDetailed : public LogDetailed{
public:
	LogInfoDetailed(const char *filename, int filenumber) : LogDetailed(filename, filenumber){
		_log_level = LogLevel::Info;
	}
};

class LogWarnDetailed : public LogDetailed{
public:
	LogWarnDetailed(const char *filename, int filenumber) : LogDetailed(filename, filenumber){
		_log_level = LogLevel::Warn;
	}
};

class LogErrorDetailed : public LogDetailed{
public:
	LogErrorDetailed(const char *filename, int filenumber) : LogDetailed(filename, filenumber){
		_log_level = LogLevel::Err;
	}
};

class LogStatusDetailed : public LogDetailed{
public:
	LogStatusDetailed(const char *filename, int filenumber) : LogDetailed(filename, filenumber){
		_log_level = LogLevel::Status;
	}
};

}; // namespace

#endif // UTILS_HPP
