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

VEIDES_RC veides_utils_topic_match(const char *sub, const char *topic)
{
    size_t spos;

    if (!sub || !topic || sub[0] == 0 || topic[0] == 0) {
        return VEIDES_RC_FAILURE;
    }

    if ((sub[0] == '$' && topic[0] != '$') || (topic[0] == '$' && sub[0] != '$')) {
        return VEIDES_RC_SUCCESS;
    }

    spos = 0;

    while (sub[0] != 0) {
        if (topic[0] == '+' || topic[0] == '#') {
            return VEIDES_RC_FAILURE;
        }
        if (sub[0] != topic[0] || topic[0] == 0) { /* Check for wildcard matches */
            if (sub[0] == '+') {
                /* Check for bad "+foo" or "a/+foo" subscription */
                if (spos > 0 && sub[-1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                /* Check for bad "foo+" or "foo+/a" subscription */
                if (sub[1] != 0 && sub[1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                spos++;
                sub++;
                while(topic[0] != 0 && topic[0] != '/') {
                    if (topic[0] == '+' || topic[0] == '#') {
                        return VEIDES_RC_FAILURE;
                    }
                    topic++;
                }
                if (topic[0] == 0 && sub[0] == 0) {
                    return VEIDES_RC_SUCCESS;
                }
            } else if (sub[0] == '#') {
                /* Check for bad "foo#" subscription */
                if (spos > 0 && sub[-1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                /* Check for # not the final character of the sub, e.g. "#foo" */
                if (sub[1] != 0) {
                    return VEIDES_RC_FAILURE;
                }

                while(topic[0] != 0) {
                    if (topic[0] == '+' || topic[0] == '#') {
                        return VEIDES_RC_FAILURE;
                    }
                    topic++;
                }
                return VEIDES_RC_SUCCESS;
            } else {
                /* Check for e.g. foo/bar matching foo/+/# */
                if (topic[0] == 0
                        && spos > 0
                        && sub[-1] == '+'
                        && sub[0] == '/'
                        && sub[1] == '#')
                {
                    return VEIDES_RC_SUCCESS;
                }

                /* There is no match at this point, but is the sub invalid? */
                while(sub[0] != 0) {
                    if (sub[0] == '#' && sub[1] != 0) {
                        return VEIDES_RC_FAILURE;
                    }
                    spos++;
                    sub++;
                }

                /* Valid input, but no match */
                return VEIDES_RC_FAILURE;
            }
        } else {
            /* sub[spos] == topic[tpos] */
            if (topic[1] == 0) {
                /* Check for e.g. foo matching foo/# */
                if (sub[1] == '/'
                        && sub[2] == '#'
                        && sub[3] == 0) {
                    return VEIDES_RC_SUCCESS;
                }
            }
            spos++;
            sub++;
            topic++;
            if (sub[0] == 0 && topic[0] == 0) {
                return VEIDES_RC_SUCCESS;
            } else if (topic[0] == 0 && sub[0] == '+' && sub[1] == 0) {
                if (spos > 0 && sub[-1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                spos++;
                sub++;
                return VEIDES_RC_SUCCESS;
            }
        }
    }
    if ((topic[0] != 0 || sub[0] != 0)) {
        return VEIDES_RC_FAILURE;
    }
    while (topic[0] != 0) {
        if (topic[0] == '+' || topic[0] == '#') {
            return VEIDES_RC_FAILURE;
        }
        topic++;
    }

    return VEIDES_RC_SUCCESS;
}