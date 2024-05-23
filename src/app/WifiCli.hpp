#ifndef WIFI_CLI_HPP
#define WIFI_CLI_HPP

#include "utilities/cli/CommandInterface.hpp"

#ifdef ENABLE_LOG_LOCAL
void log_local(const char* format, ...);
#define LOG_LOCAL(x, ...) log_local(x, ##__VA_ARGS__)
#else
#define LOG_LOCAL(x, ...)
#endif
extern Cli::CommandInterface* pCli;

#endif