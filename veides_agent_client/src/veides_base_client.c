#include <MQTTAsync.h>
#include <pthread.h>

#include "veides_utils.h"
#include "veides_logger.h"
#include "veides_internal.h"

static pthread_mutex_t veides_client_mutex_store = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t* veides_client_mutex = &veides_client_mutex_store;
static int veides_mutex_inited = 0;

static void veides_init_mutex(void) {
    if (veides_mutex_inited == 1) {
        return;
    }

    int rc = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if ((rc = pthread_mutex_init(veides_client_mutex, &attr)) != 0)
        VEIDES_LOG_ERROR("Failed to initialize veides_client_mutex (rc=%d)", rc);

    veides_mutex_inited = 1;
}

static int veides_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
static int veides_properties_validate(VeidesAgentClientProperties *properties);

static VEIDES_RC veides_add_handler(VeidesHandlers *handlers, VeidesHandler *handler) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesHandler **tmp = NULL;
    tmp = realloc(handlers->entries, sizeof(VeidesHandler *));
    if (tmp == NULL) {
        return VEIDES_RC_NOMEM;
    }

    handlers->entries = tmp;

    handlers->entries[handlers->count] = handler;

    handlers->count++;

    return rc;
}

static VEIDES_RC veides_add_actionHandler(VeidesActionHandlers *handlers, VeidesActionHandler *handler) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesActionHandler **tmp = NULL;
    tmp = realloc(handlers->entries, sizeof(VeidesActionHandler *));
    if (tmp == NULL) {
        return VEIDES_RC_NOMEM;
    }

    handlers->entries = tmp;

    handlers->entries[handlers->count] = handler;

    if (handler->name == NULL) {
        handlers->anyActionId = handlers->count;
    }

    handlers->count++;

    return rc;
}

static VEIDES_RC veides_add_methodHandler(VeidesMethodHandlers *handlers, VeidesMethodHandler *handler) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesMethodHandler **tmp = NULL;
    tmp = realloc(handlers->entries, sizeof(VeidesMethodHandler *));
    if (tmp == NULL) {
        return VEIDES_RC_NOMEM;
    }

    handlers->entries = tmp;

    handlers->entries[handlers->count] = handler;

    if (handler->name == NULL) {
        handlers->anyMethodId = handlers->count;
    }

    handlers->count++;

    return rc;
}

static VeidesHandler* veides_client_getHandler(VeidesHandlers *handlers, char *topic) {
    int i = 0;
    VeidesHandler *handler = NULL;
    int found = 0;
    for (i = 0; i < handlers->count; i++) {
        handler = handlers->entries[i];
        if (topic && handler->topic && veides_utils_topic_match(handler->topic, topic) == VEIDES_RC_SUCCESS) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        handler = NULL;
    }

    return handler;
}

VeidesActionHandler* veides_client_getActionHandler(VeidesActionHandlers *handlers, char *name) {
    VeidesActionHandler *handler = NULL;

    if (!handlers) {
        VEIDES_LOG_ERROR("Invalid handle for action handlers provided (handlers=%s)", handlers ? "valid" : "NULL");
        return NULL;
    }

    if (!name || *name == '\0') {
        VEIDES_LOG_ERROR("Invalid arguments provided (name=%s)", name ? "Valid" : "NULL");
        return NULL;
    }

    int i = 0;
    int found = 0;
    for (i = 0; i < handlers->count; i++) {
        handler = handlers->entries[i];
        if (handler->name && strcmp(name, handler->name) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        handler = NULL;
    }

    return handler;
}

VeidesActionHandler* veides_client_getAnyActionHandler(VeidesActionHandlers *handlers) {
    VeidesActionHandler *handler = NULL;

    if (!handlers) {
        VEIDES_LOG_ERROR("Invalid handle for action handlers provided (handlers=%s)", handlers ? "valid" : "NULL");
        return NULL;
    }

    VEIDES_LOG_DEBUG("Got any action id %d", handlers->anyActionId);

    if (handlers->anyActionId != -1) {
        handler = handlers->entries[handlers->anyActionId];
    } else {
        handler = NULL;
    }

    return handler;
}

VeidesMethodHandler* veides_client_getMethodHandler(VeidesMethodHandlers *handlers, char *name) {
    VeidesMethodHandler *handler = NULL;

    if (!handlers) {
        VEIDES_LOG_ERROR("Invalid handle for method handlers provided (handlers=%s)", handlers ? "valid" : "NULL");
        return NULL;
    }

    if (!name || *name == '\0') {
        VEIDES_LOG_ERROR("Invalid arguments provided (name=%s)", name ? "Valid" : "NULL");
        return NULL;
    }

    int i = 0;
    int found = 0;
    for (i = 0; i < handlers->count; i++) {
        handler = handlers->entries[i];
        if (handler->name && strcmp(name, handler->name) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        handler = NULL;
    }

    return handler;
}

VeidesMethodHandler* veides_client_getAnyMethodHandler(VeidesMethodHandlers *handlers) {
    VeidesMethodHandler *handler = NULL;

    if (!handlers) {
        VEIDES_LOG_ERROR("Invalid handle for method handlers provided (handlers=%s)", handlers ? "valid" : "NULL");
        return NULL;
    }

    VEIDES_LOG_DEBUG("Got any method id %d", handlers->anyMethodId);

    if (handlers->anyMethodId != -1) {
        handler = handlers->entries[handlers->anyMethodId];
    } else {
        handler = NULL;
    }

    return handler;
}

void onConnect(void *context, MQTTAsync_successData *response) {
    VeidesClient *client = (VeidesClient *) context;

    pthread_mutex_lock(veides_client_mutex);

    client->connected = 1;

    VEIDES_LOG_INFO("Veides Agent client connected");

    pthread_mutex_unlock(veides_client_mutex);
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    int rc = 0;

    VeidesClient *client = (VeidesClient *) context;

    if (response) {
        rc = response->code;
    }

    pthread_mutex_lock(veides_client_mutex);

    client->connected = (0 - rc);

    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to connect (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to connect");
    }

    pthread_mutex_unlock(veides_client_mutex);
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    VeidesClient *client = (VeidesClient *) context;

    pthread_mutex_lock(veides_client_mutex);

    client->connected = 0;

    VEIDES_LOG_INFO("Client disconnected successfuly");

    pthread_mutex_unlock(veides_client_mutex);
}

void onDisconnectFailure(void *context, MQTTAsync_failureData *response) {
    int rc = 0;

    VeidesClient *client = (VeidesClient *) context;

    if (response) {
        rc = response->code;
    }

    pthread_mutex_lock(veides_client_mutex);

    client->connected = (0 - rc);

    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to disconnect (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to disconnect");
    }

    pthread_mutex_unlock(veides_client_mutex);
}

void onSendFailure(void *context, MQTTAsync_failureData *response) {
    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to send message (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to send message");
    }
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    VEIDES_LOG_DEBUG("Subscribed to a topic");
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to subscribe to a topic (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to subscribe to a topic");
    }
}

VEIDES_RC veides_client_create(void **veidesClient, VeidesAgentClientProperties *properties) {
    veides_utils_writeClientVersion();

    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    veides_init_mutex();

    if (!veidesClient) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid aget client handle provided (rc=%d)", rc);
        return rc;
    }

    if (*veidesClient != NULL) {
        rc = VEIDES_RC_INVALID_PARAM_VALUE;
        VEIDES_LOG_ERROR("Agent client handle is already created");
        return rc;
    }

    rc = veides_properties_validate(properties);

    if (rc != VEIDES_RC_SUCCESS) {
        return rc;
    }

    char *host = properties->connectionProperties->host;
    int port = properties->connectionProperties->port;

    int len = strlen(host) + 13;
    char url[len];

    if (port == 1883) {
        snprintf(url, len, "tcp://%s:%d", host, port);
    } else {
        snprintf(url, len, "ssl://%s:%d", host, port);
    }

    VeidesClient *client = (VeidesClient *) calloc(1, sizeof(VeidesClient));
    client->properties = properties;
    client->clientId = properties->agentProperties->clientId;
    client->mqttClient = NULL;
    client->handlers = (VeidesHandlers *) calloc(1, sizeof(VeidesHandlers));
    client->handlers->count = 0;
    client->actionHandlers = (VeidesActionHandlers *) calloc(1, sizeof(VeidesActionHandlers));
    client->actionHandlers->count = 0;
    client->actionHandlers->anyActionId = -1;
    client->methodHandlers = (VeidesMethodHandlers *) calloc(1, sizeof(VeidesMethodHandlers));
    client->methodHandlers->count = 0;
    client->methodHandlers->anyMethodId = -1;

    MQTTAsync mqttClient;
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;

    create_opts.MQTTVersion = MQTTVERSION_3_1_1;
    create_opts.sendWhileDisconnected = 1;

    rc = MQTTAsync_createWithOptions(&mqttClient, url, client->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    if (rc != MQTTASYNC_SUCCESS) {
        VEIDES_LOG_ERROR("MQTTAsync_createWithOptions failed (clientId=%s, url=%s)", client->clientId, url);
        veides_client_destroy(client);
        client = NULL;
        return rc;
    }

    veides_utils_sleep(50);

    client->mqttClient = (void *) mqttClient;

    *veidesClient = client;

    return rc;
}

VEIDES_RC veides_client_destroy(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;
    VeidesHandlers *handlers = NULL;
    int i = 0;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid or NULL agent client handle provided (rc=%d)", rc);
        return rc;
    }
    veides_utils_freePtr((void *) client->clientId);
    handlers = client->handlers;

    for (i=0; i<handlers->count; i++)
    {
        VeidesHandler *sub = handlers->entries[i];
        veides_utils_freePtr((void *) sub->topic);
    }

    veides_utils_freePtr((void *) handlers);

    client->properties = NULL;
    veides_utils_freePtr((void *) client);
    client = NULL;

    return rc;
}

VEIDES_RC veides_client_connect(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    VeidesAgentClientProperties *properties = (VeidesAgentClientProperties *) client->properties;

    rc = veides_properties_validate(properties);

    if (rc != VEIDES_RC_SUCCESS) {
        return rc;
    }

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;

    int port = client->properties->connectionProperties->port;

    conn_opts.keepAliveInterval = 60;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
    conn_opts.context = client;
    conn_opts.automaticReconnect = 1;
    conn_opts.cleansession = 1;

    if (port == 8883) {
        ssl_opts.enableServerCertAuth = 1;
        conn_opts.ssl = &ssl_opts;

        conn_opts.username = properties->agentProperties->key;
        conn_opts.password = properties->agentProperties->secretKey;
    }

    MQTTAsync_setCallbacks((MQTTAsync *) client->mqttClient, (void *) client, NULL, veides_client_messageArrived, NULL);

    VEIDES_LOG_DEBUG("Connecting to %s with clientId %s", client->properties->connectionProperties->host, client->clientId);

    if ((rc = MQTTAsync_connect((MQTTAsync *) client->mqttClient, &conn_opts)) == MQTTASYNC_SUCCESS) {
        int iteration = 1;
        int isConnected = 0;
        while (isConnected == 0) {
            veides_utils_sleep(1000);

            pthread_mutex_lock(veides_client_mutex);
            isConnected = client->connected;
            pthread_mutex_unlock(veides_client_mutex);

            if (isConnected == 1) {
                break;
            }

            if (isConnected < 0) {
                rc = (0 - isConnected);
                pthread_mutex_lock(veides_client_mutex);
                client->connected = 0;
                pthread_mutex_unlock(veides_client_mutex);
                break;
            }

            VEIDES_LOG_INFO("Waiting for client to connect (iteration=%d)", iteration);

            if (iteration >= 30)  {
                rc = VEIDES_RC_TIMEOUT;
                break;
            }

            iteration++;
        }

    }

    return rc;
}

VEIDES_RC veides_client_isConnected(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (MQTTAsync_isConnected(client->mqttClient) == 0) {
        rc = VEIDES_RC_NOT_CONNECTED;
    }

    return rc;
}

VEIDES_RC veides_client_disconnect(void *veidesClient) {
    VEIDES_RC rc = 0;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    MQTTAsync mqttClient = (MQTTAsync *) client->mqttClient;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

    disc_opts.onSuccess = onDisconnect;
    disc_opts.onFailure = onDisconnectFailure;
    disc_opts.context = client;

    int iteration = 1;
    int isConnected = client->connected;

    if (isConnected == 1) {
        VEIDES_LOG_INFO("Disconnect client.");
        int mqttRC = 0;
        mqttRC = MQTTAsync_disconnect(mqttClient, &disc_opts);
        if (mqttRC == MQTTASYNC_DISCONNECTED) {
            rc = VEIDES_RC_SUCCESS;
        } else {
            rc = mqttRC;
            return rc;
        }

        pthread_mutex_lock(veides_client_mutex);
        isConnected = client->connected;
        pthread_mutex_unlock(veides_client_mutex);

        while (isConnected == 1) {
            veides_utils_sleep(1000);

            pthread_mutex_lock(veides_client_mutex);
            isConnected = client->connected;
            pthread_mutex_unlock(veides_client_mutex);

            if (isConnected == 0) {
                break;
            }

            if (isConnected < 0) {
                rc = (0 - isConnected);
                pthread_mutex_lock(veides_client_mutex);
                client->connected = 0;
                pthread_mutex_unlock(veides_client_mutex);
                break;
            }

            VEIDES_LOG_INFO("Wait for client to disconnect (iteration=%d)", iteration);

            if (iteration >= 30)  {
                rc = VEIDES_RC_TIMEOUT;
                break;
            }

            iteration++;
        }
    }

    return rc;
}

static int reconnect_delay(int try) {
    if (try <= 10) {
        return 3000;
    }

    if (try <= 20) {
        return 15000;
    }

    return 60000;
}

VEIDES_RC veides_client_retry_connection(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    int retry = 1;

    while((rc = veides_client_connect(veidesClient)) != MQTTASYNC_SUCCESS)
    {
        int delay = reconnect_delay(retry++);
        VEIDES_LOG_DEBUG("Connect retry (attempt=%d, delay=%d)", retry, delay);
        veides_utils_sleep(delay);
    }

    return rc;
}

VEIDES_RC veides_client_publish(void *veidesClient, char *topic, char *payload, int qos) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (client->connected == 0) {
        rc = VEIDES_RC_NOT_CONNECTED;
        VEIDES_LOG_ERROR("Agent client is not connected");
        return rc;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync mqttClient = (MQTTAsync *) client->mqttClient;

    opts.onFailure = onSendFailure;
    opts.context = client;

    int payloadlen = 0;
    if (payload && *payload != '\0') {
        payloadlen = strlen(payload);
    }

    VEIDES_LOG_DEBUG("Sending message on topic %s with payload %s", topic, payload);

    rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    if (rc != MQTTASYNC_SUCCESS && rc != VEIDES_RC_INVALID_HANDLE) {
        VEIDES_LOG_ERROR("MQTTAsync_send returned error (rc=%d)", rc);
        VEIDES_LOG_WARNING("Connection is lost, retry connection and republish message.");
        veides_client_retry_connection(mqttClient);
        rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    }

    return rc;
}

VEIDES_RC veides_client_subscribe(void *veidesClient, char *topic, int qos) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    for (int tries = 0; tries < 5; tries++) {
        if (client->connected == 0) {
            VEIDES_LOG_WARNING("Client is not connected yet. Wait for client to connect and subscribe.");
            veides_utils_sleep(2000);
        }
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync mqttClient = (MQTTAsync *) client->mqttClient;

    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;

    rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts);

    if (rc != MQTTASYNC_SUCCESS) {
        VEIDES_LOG_ERROR("Unable to subscribe to %s", topic);
    }

    return rc;
}

VEIDES_RC veides_client_setHandler(void *veidesClient, char *topic, VeidesCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!topic || *topic == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid topic provided (rc=%d)", rc);
        return rc;
    }

    int found = 0;
    int i=0;
    for (i = 0; i < client->handlers->count; i++) {
        VeidesHandler *handler = client->handlers->entries[i];
        if (topic && handler->topic && veides_utils_topic_match(handler->topic, topic) == VEIDES_RC_SUCCESS) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        VeidesHandler *handler = (VeidesHandler *) calloc(1, sizeof(VeidesHandler));
        handler->topic = strdup(topic);
        handler->callback = callback;
        rc = veides_add_handler(client->handlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_DEBUG("Set message handler for topic %s", topic);
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for topic %s (rc=%d)", topic, rc);
        }
    } else {
        VeidesHandler *handler = client->handlers->entries[i];
        handler->callback = callback;
        VEIDES_LOG_DEBUG("Updated handler for topic %s", topic);
    }

    return rc;
}

VEIDES_RC veides_client_setActionHandler(void *veidesClient, char *name, VeidesActionCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    int found = 0;
    int i = 0;
    for (i = 0; i < client->actionHandlers->count; i++) {
        VeidesActionHandler *handler = client->actionHandlers->entries[i];
        if (handler->name && strcmp(name, handler->name) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        VeidesActionHandler *handler = (VeidesActionHandler *) calloc(1, sizeof(VeidesActionHandler));
        handler->name = strdup(name);
        handler->callback = callback;

        rc = veides_add_actionHandler(client->actionHandlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_INFO("Added handler for action %s", name);
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for action %s (rc=%d)", name, rc);
        }
    } else {
        VeidesActionHandler *handler = client->actionHandlers->entries[i];
        handler->callback = callback;
        VEIDES_LOG_INFO("Updated handler for action %s", name);
    }

    return rc;
}

VEIDES_RC veides_client_setAnyActionHandler(void *veidesClient, VeidesAnyActionCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (client->actionHandlers->anyActionId == -1) {
        VeidesActionHandler *handler = (VeidesActionHandler *) calloc(1, sizeof(VeidesActionHandler));
        handler->name = NULL;
        handler->callback = callback;

        rc = veides_add_actionHandler(client->actionHandlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_INFO("Added handler for any action");
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for any action (rc=%d)", rc);
        }
    } else {
        VeidesActionHandler *handler = client->actionHandlers->entries[client->actionHandlers->anyActionId];
        handler->callback = callback;
        VEIDES_LOG_INFO("Updated handler for any action");
    }

    return rc;
}

VEIDES_RC veides_client_setMethodHandler(void *veidesClient, char *name, VeidesMethodCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    int found = 0;
    int i = 0;
    for (i = 0; i < client->methodHandlers->count; i++) {
        VeidesMethodHandler *handler = client->methodHandlers->entries[i];
        if (handler->name && strcmp(name, handler->name) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        VeidesMethodHandler *handler = (VeidesMethodHandler *) calloc(1, sizeof(VeidesMethodHandler));
        handler->name = strdup(name);
        handler->callback = callback;

        rc = veides_add_methodHandler(client->methodHandlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_INFO("Added handler for method %s", name);
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for method %s (rc=%d)", name, rc);
        }
    } else {
        VeidesMethodHandler *handler = client->methodHandlers->entries[i];
        handler->callback = callback;
        VEIDES_LOG_INFO("Updated handler for method %s", name);
    }

    return rc;
}

VEIDES_RC veides_client_setAnyMethodHandler(void *veidesClient, VeidesAnyMethodCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (client->methodHandlers->anyMethodId == -1) {
        VeidesMethodHandler *handler = (VeidesMethodHandler *) calloc(1, sizeof(VeidesMethodHandler));
        handler->name = NULL;
        handler->callback = callback;

        rc = veides_add_methodHandler(client->methodHandlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_INFO("Added handler for any method");
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for any method (rc=%d)", rc);
        }
    } else {
        VeidesMethodHandler *handler = client->methodHandlers->entries[client->methodHandlers->anyMethodId];
        handler->callback = callback;
        VEIDES_LOG_INFO("Updated handler for any method");
    }

    return rc;
}

static int veides_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesClient *client = (VeidesClient *) context;

    if (topicLen > 0) {
        VEIDES_LOG_DEBUG("Message Received (topic=%s, topicLen=%d)", topicName ? topicName : "", topicLen);
    }

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (client->handlers->count == 0) {
        rc = VEIDES_RC_HANDLER_NOT_FOUND;
        VEIDES_LOG_WARNING("Not a single callback is registered");
        return rc;
    }

    VeidesHandler *sub = veides_client_getHandler(client->handlers, topicName);
    if (sub == NULL) {
        rc = VEIDES_RC_HANDLER_NOT_FOUND;
        VEIDES_LOG_ERROR("Callback not found for topic %s", topicName ? topicName : "");
        return rc;
    }

    VeidesCallbackHandler cb = (VeidesCallbackHandler) sub->callback;

    if (cb != 0) {
        void *payload = message->payload;
        size_t payloadlen = message->payloadlen;
        char *pl = (char *) payload;

        pl[payloadlen] = '\0';
        VEIDES_LOG_DEBUG("Invoke callback to process message (topic=%s)", topicName);
        (*cb)(context, topicName, topicLen, payload, payloadlen);
    } else {
        VEIDES_LOG_DEBUG("No registered callback function is found to process the arrived message.");
    }

    if (rc == VEIDES_RC_SUCCESS) {
        return MQTTASYNC_TRUE;
    }

    return rc;
}

static int veides_properties_validate(VeidesAgentClientProperties *properties) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid properties handle (rc=%d)", rc);
        return rc;
    }

    if (properties->agentProperties == NULL || properties->connectionProperties == NULL) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid agent or connection properties provided (rc=%d)", rc);
        return rc;
    }

    char *clientHost = NULL;
    clientHost = properties->connectionProperties->host;

    int port = properties->connectionProperties->port;

    if (clientHost == NULL || *clientHost == '\0' || !port) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid connection properties provided (rc=%d)", rc);
        return rc;
    }

    char *agentClientId = NULL;
    char *agentKey = NULL;
    char *agentSecretKey = NULL;

    agentClientId = properties->agentProperties->clientId;
    agentKey = properties->agentProperties->key;
    agentSecretKey = properties->agentProperties->secretKey;

    if (agentClientId == NULL || *agentClientId == '\0'
        || agentKey == NULL || *agentKey == '\0'
        || agentSecretKey == NULL || *agentSecretKey == '\0'
    ) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid agent properties provided (rc=%d)", rc);
        return rc;
    }

    return rc;
}