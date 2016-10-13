#ifndef __LOGGING_H__
#define __LOGGING_H__

namespace logging {

enum eLogCategory { LC_DEBUG, LC_INFO, LC_WARNING, LC_ERROR, NUM_LC_LOGCATEGORY };

void logmsg(eLogCategory lc, const char* file, int line, const char* fmt, ...);

}

#define log_debug(...) logging::logmsg(logging::LC_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) logging::logmsg(logging::LC_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warning(...) logging::logmsg(logging::LC_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logging::logmsg(logging::LC_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif // __LOGGING_H__
