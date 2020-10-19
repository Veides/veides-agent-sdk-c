#if !defined(VEIDES_INTERNAL_H)
#define VEIDES_INTERNAL_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

#include <MQTTProperties.h>

#include "veides_rc.h"
#include "veides_properties.h"
#include "veides_base_client.h"
#include "veides_utils.h"
#include "veides_logger.h"

typedef struct VeidesClient {
    VeidesAgentClientProperties *properties;
    char              *clientId;
    void              *mqttClient;
    VeidesHandlers    *handlers;
    VeidesActionHandlers    *actionHandlers;
    int               connected;
} VeidesClient;

VEIDES_RC veides_client_create(void **client, VeidesAgentClientProperties *properties);
VEIDES_RC veides_client_destroy(void *client);
VEIDES_RC veides_client_connect(void *client);
VEIDES_RC veides_client_isConnected(void *client);
VEIDES_RC veides_client_disconnect(void *client);
VEIDES_RC veides_client_retry_connection(void *client);
VEIDES_RC veides_client_setHandler(void *client, char * topic, VeidesCallbackHandler handler);
VEIDES_RC veides_client_setActionHandler(void *client, char *name, VeidesActionCallbackHandler callback);
VEIDES_RC veides_client_setAnyActionHandler(void *client, VeidesAnyActionCallbackHandler callback);
VeidesActionHandler* veides_client_getActionHandler(VeidesActionHandlers *handlers, char *name);
VeidesActionHandler* veides_client_getAnyActionHandler(VeidesActionHandlers *handlers);
VEIDES_RC veides_client_subscribe(void *client, char *topic, int qos);
VEIDES_RC veides_client_publish(void *client, char *topic, char *payload, int qos);


#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_INTERNAL_H */
