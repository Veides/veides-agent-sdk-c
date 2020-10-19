#include "veides_version.h"
#include "veides_utils.h"
#include "veides_logger.h"

void veides_utils_freePtr(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
        ptr = NULL;
    }
}

void veides_utils_sleep(long milsecs) {
    usleep(milsecs * 1000);
}

void veides_utils_writeClientVersion(void)
{
    VEIDES_LOG_INFO("Veides Agent Client - Version: %s", VEIDES_AGENT_CLIENT_VERSION);
    VEIDES_LOG_INFO("Paho MQTT Client - Version: %s", PAHO_CLIENT_VERSION);
}
