#ifndef CRDRDRD
#define CRDRDRD

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>

#define CRDRDRD_SYSLOG_FACILITY	200				// this value will identify messages coming from the crdrdrd
#define CRDRDRD_LOG_LEVEL			LOG_INFO		// specify the current log level for the crdrdrd
#define CRDRDRD_APP_NAME			"crdrdrd"		// perhaps this should be specified elsewhere
#define CRDRDRD_PID_FILE "/var/run/mxn/crdrdrd.pid"


void daemonize(const char *cmd);							// perform relevant actions to make the program a daemon
void startSwipeListener();	// loop waiting for card swipes

#endif
