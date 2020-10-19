#if !defined(VEIDES_CONFIG_H)
#define VEIDES_CONFIG_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include <stdio.h>
#include "veides_rc.h"
#include "veides_utils.h"
#include "veides_logger.h"

typedef struct VeidesAgentProperties {
    char *clientId;
    char *key;
    char *secretKey;
} VeidesAgentProperties;

typedef struct VeidesConnectionProperties {
    char *host;
    int port;
} VeidesConnectionProperties;

typedef struct VeidesAgentClientProperties {
    VeidesAgentProperties *agentProperties;
    VeidesConnectionProperties *connectionProperties;
    int logLevel;
    int mqttLogLevel;
} VeidesAgentClientProperties;

/**
 * @brief Creates VeidesAgentClientProperties object that stores agent and connection properties used by VeidesAgentClent.
 * To destroy object use VeidesAgentClientProperties_destroy()
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesAgentClientProperties_create(VeidesAgentClientProperties **properties);

/**
 * @brief Destroys VeidesAgentClientProperties object created by VeidesAgentClientProperties_create()
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesAgentClientProperties_destroy(VeidesAgentClientProperties *properties);

/**
 * @brief Sets VeidesAgentClientProperties properties provided in environment variables
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesAgentClientProperties_setPropertiesFromEnv(VeidesAgentClientProperties *properties);

/**
 * @brief Sets VeidesAgentClientProperties property with given name
 *
 * @param properties
 * @param name
 * @param value
 */
LIBRARY_API VEIDES_RC VeidesAgentClientProperties_setProperty(VeidesAgentClientProperties *properties, const char *name, const char *value);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_CONFIG_H */
