#include "util.h"

#include <iostream>
#include <stdexcept>

#include "util.h"

using namespace std;

namespace rpulp {
namespace tempest {

void Log::debug(const string& msg) {
    Log::log("DEBUG", msg);
}

void Log::info(const string& msg) {
    Log::log("INFO", msg);
}

void Log::error(const string& msg) {
    Log::log("ERROR", msg);
}

void Log::log(const string& level, const string& msg) {
    cout << level << ": " << msg << endl;
}

std::exception throw_runtime_error(const string& msg) {
    Log::error(msg);
    throw std::runtime_error(msg);
}


}}
