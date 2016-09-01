#ifndef RPULP_TEMPEST_UTIL_H
#define RPULP_TEMPEST_UTIL_H

#include <iostream>
#include <string>


namespace rpulp { namespace tempest {

/**
 */
class Log {
public:
	static void debug(const std::string& msg);
	
	static void info(const std::string& msg);
	
	static void error(const std::string& msg);
	
private:

	static void log(const std::string& level, const std::string& msg);

	Log() = delete;
	Log(const Log& that) = delete;
	Log& operator=(const Log& that) = delete;
};

std::exception throw_runtime_error(const std::string& msg);

}}

#endif