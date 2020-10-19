#if !defined(VEIDES_LOGGER_H)
#define VEIDES_LOGGER_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "veides_rc.h"

typedef enum VeidesLogLevel {
    VEIDES_LOG_DEBUG = 0,
    VEIDES_LOG_INFO = 1,
    VEIDES_LOG_WARNING = 2,
    VEIDES_LOG_ERROR = 3
} VeidesLogLevel;

typedef void VeidesLogHandler(int logLevel, char *message);

void veides_log(VeidesLogLevel level, const char *items, ...);
void veides_log_setLevel(VeidesLogLevel level);
VEIDES_RC veides_log_setHandler(VeidesLogHandler *handler);

#define VEIDES_LOG_DEBUG(fmts...) veides_log(VEIDES_LOG_DEBUG, fmts);
#define VEIDES_LOG_INFO(fmts...) veides_log(VEIDES_LOG_INFO, fmts);
#define VEIDES_LOG_WARNING(fmts...) veides_log(VEIDES_LOG_WARNING, fmts);
#define VEIDES_LOG_ERROR(fmts...) veides_log(VEIDES_LOG_ERROR, fmts);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_LOGGER_H */
