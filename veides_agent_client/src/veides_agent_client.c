#include "veides_agent_client.h"
#include "veides_base_client.h"
#include "veides_internal.h"
#include "cJSON/cJSON.h"

static void veides_client_actionMessageReceived(VeidesAgentClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen);
static void veides_client_methodMessageReceived(VeidesAgentClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen);

static char* veides_client_buildTopic(char *clientId, char *type) {
    char *format = "agent/%s/%s";
    int len = strlen(format) + strlen(clientId) + strlen(type) - 2;

    char data[len];
    snprintf(data, len, format, clientId, type);

    return strdup(data);
}

static char* veides_client_buildTopicWithName(char *clientId, char *type, char *name) {
    char *format = "agent/%s/%s/%s";
    int len = strlen(format) + strlen(clientId) + strlen(type) + strlen(name) - 3;

    char data[len];
    snprintf(data, len, format, clientId, type, name);

    return strdup(data);
}

VEIDES_RC VeidesAgentClient_create(VeidesAgentClient **client, VeidesAgentClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_client_create((void **) client, properties);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to create agent handle (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_destroy(VeidesAgentClient *client) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_client_destroy((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to destroy agent handle (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_connect(VeidesAgentClient *client) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_client_connect((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to connect (rc=%d)", rc);
    }

    if (rc == VEIDES_RC_SUCCESS) {
        VeidesClient *veidesClient = (VeidesClient *) client;

        char *topic = veides_client_buildTopic(veidesClient->clientId, "action_received");

        VEIDES_LOG_DEBUG("Subscribe to topic %s", topic);

        rc = veides_client_subscribe((void *) client, topic, 1);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to subscribe to topic %s (rc=%d)", topic, rc);
        }

        rc = veides_client_setHandler((void *) client, topic, (void *) veides_client_actionMessageReceived);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to set inernal actions handler (rc=%d)", rc);
        }

        topic = veides_client_buildTopicWithName(veidesClient->clientId, "method", "+");

        rc = veides_client_subscribe((void *) client, topic, 1);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to subscribe to topic %s (rc=%d)", topic, rc);
        }

        rc = veides_client_setHandler((void *) client, topic, (void *) veides_client_methodMessageReceived);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to set inernal methods handler (rc=%d)", rc);
        }
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_disconnect(VeidesAgentClient *client) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_client_disconnect((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to disconnect (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_sendMethodResponse(VeidesAgentClient *client, char *name, char *payload, int code) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name provided (rc=%d)", rc);
        return rc;
    }

    if (!payload || *payload == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid payload provided (rc=%d)", rc);
        return rc;
    }

    VeidesClient *veidesClient = (VeidesClient *) client;

    if (veidesClient->clientId == NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    char *topic = veides_client_buildTopicWithName(veidesClient->clientId, "method_response", name);

    char *format = "{\"payload\": %s, \"code\": %d}";
    int len = strlen(format) + strlen(payload) + sizeof(int) - 2;
    char response[len];
    snprintf(response, len, format, payload, code);

    return veides_client_publish((void *) client, topic, response, 1);
}

VEIDES_RC VeidesAgentClient_sendActionCompleted(VeidesAgentClient *client, char *name) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name provided (rc=%d)", rc);
        return rc;
    }

    VeidesClient *veidesClient = (VeidesClient *) client;

    if (veidesClient->clientId == NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    char *topic = veides_client_buildTopic(veidesClient->clientId, "action_completed");

    char *format = "{\"name\": \"%s\"}";
    int len = strlen(format) + strlen(name) - 1;
    char payload[len];
    snprintf(payload, len, format, name);

    return veides_client_publish((void *) client, topic, payload, 1);
}

VEIDES_RC VeidesAgentClient_sendEvent(VeidesAgentClient *client, char *name) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name provided (rc=%d)", rc);
        return rc;
    }

    VeidesClient *veidesClient = (VeidesClient *) client;

    if (veidesClient->clientId == NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    char *topic = veides_client_buildTopic(veidesClient->clientId, "event");

    char *format = "{\"name\": \"%s\"}";
    int len = strlen(format) + strlen(name) - 1;
    char payload[len];
    snprintf(payload, len, format, name);

    return veides_client_publish((void *) client, topic, payload, 1);
}

VEIDES_RC VeidesAgentClientFacts_create(VeidesFacts **facts) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!facts) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid facts handle provided (rc=%d)", rc);
        return rc;
    }
    if (*facts != NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Facts object is already initialized (rc=%d)", rc);
        return rc;
    }

    *facts = (VeidesFacts *) calloc(1, sizeof(VeidesFacts *));

    if (*facts == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    (*facts)->count = 0;

    return rc;
}

VEIDES_RC VeidesAgentClientFacts_destroy(VeidesFacts *facts) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!facts) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid facts handle provided (rc=%d)", rc);
    }

    for (int i = 0; i < facts->count; i++) {
        veides_utils_freePtr((void *) facts->entries[i]->name);
        veides_utils_freePtr((void *) facts->entries[i]->value);
    }

    veides_utils_freePtr(facts);

    return rc;
}

VEIDES_RC VeidesAgentClientFacts_addFact(VeidesFacts *facts, char *name, char *value) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesFact **tmp = NULL;
    tmp = realloc(facts->entries, sizeof(VeidesFact *));
    if (tmp == NULL) {
        return VEIDES_RC_NOMEM;
    }

    facts->entries = tmp;

    VeidesFact *fact = (VeidesFact *) malloc(sizeof(VeidesFact *));
    fact->name = strdup(name);
    fact->value = strdup(value);

    facts->entries[facts->count] = fact;

    facts->count++;

    return rc;
}

VEIDES_RC VeidesAgentClient_sendFacts(VeidesAgentClient *client, VeidesFacts *facts) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!facts) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid facts provided (rc=%d)", rc);
        return rc;
    }

    VeidesClient *veidesClient = (VeidesClient *) client;

    if (veidesClient->clientId == NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    char *topic = veides_client_buildTopic(veidesClient->clientId, "facts");

    cJSON *payload = cJSON_CreateObject();

    if (!payload) {
        rc = VEIDES_RC_NOMEM;
        VEIDES_LOG_WARNING("Unable to create facts payload object (rc=%d)", rc);
        return rc;
    }

    for (int i = 0; i < facts->count; i++) {
        cJSON *value = cJSON_CreateString(facts->entries[i]->value);

        if (!value) {
            continue;
        }

        cJSON_AddItemToObject(payload, facts->entries[i]->name, value);
    }

    char *data = cJSON_PrintUnformatted(payload);

    rc = veides_client_publish((void *) client, topic, data, 1);

    cJSON_Delete(payload);

    return rc;
}

VEIDES_RC VeidesAgentClient_sendTrail(VeidesAgentClient *client, char *name, char *value) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name provided (rc=%d)", rc);
        return rc;
    }

    VeidesClient *veidesClient = (VeidesClient *) client;

    if (veidesClient->clientId == NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    char *topic = veides_client_buildTopicWithName(veidesClient->clientId, "trail", name);

    char *format = "{\"value\": \"%s\"}";
    int len = strlen(format) + strlen(value) - 1;
    char payload[len];
    snprintf(payload, len, format, value);

    return veides_client_publish((void *) client, topic, payload, 1);
}

VEIDES_RC VeidesAgentClient_setActionHandler(VeidesAgentClient *client, char *name, VeidesActionCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!callback || !name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name or callback provided (rc=%d)", rc);
        return rc;
    }

    rc = veides_client_setActionHandler((void *) client, name, callback);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to set action handler (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_setAnyActionHandler(VeidesAgentClient *client, VeidesAnyActionCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!callback) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid callback provided (rc=%d)", rc);
        return rc;
    }

    rc = veides_client_setAnyActionHandler((void *) client, callback);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to set any action handler (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_setMethodHandler(VeidesAgentClient *client, char *name, VeidesMethodCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!callback || !name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name or callback provided (rc=%d)", rc);
        return rc;
    }

    rc = veides_client_setMethodHandler((void *) client, name, callback);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to set method handler (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_setAnyMethodHandler(VeidesAgentClient *client, VeidesAnyMethodCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid agent client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!callback) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid callback provided (rc=%d)", rc);
        return rc;
    }

    rc = veides_client_setAnyMethodHandler((void *) client, callback);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to set any method handler (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesAgentClient_setLogHandler(VeidesLogHandler *handler) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_log_setHandler(handler);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to set log handler (rc=%d)", rc);
    }

    return rc;
}

void VeidesAgentClient_setLogLevel(int level) {
    veides_log_setLevel(level);
}

static void veides_client_actionMessageReceived(VeidesAgentClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen) {
    char *pl = NULL;

    pl = (char *) malloc(payloadlen+1);

    memset(pl, 0, payloadlen+1);
    strncpy(pl, payload, payloadlen);

    cJSON *json = cJSON_ParseWithLength(pl, payloadlen+1);

    if (json == NULL) {
        goto endActionReceived;
    }

    char *name = cJSON_GetObjectItemCaseSensitive(json, "name")->valuestring;

    VEIDES_LOG_DEBUG("Agent action received (name: %s)", name);

    cJSON *entities = cJSON_GetObjectItemCaseSensitive(json, "entities");

    int size = cJSON_GetArraySize(entities);

    VeidesActionEntity *actionEntities = NULL;

    if (size > 0) {
        actionEntities = (VeidesActionEntity *) malloc(size * sizeof(VeidesActionEntity *));

        if (actionEntities == NULL) {
            VEIDES_LOG_ERROR("Unable to allocate memory for action entities.");
            goto endActionReceived;
        }

        for (int i = 0; i < size; i++) {
            cJSON *actionEntity = cJSON_GetArrayItem(entities, i);

            if (!cJSON_IsObject(actionEntity)) {
                continue;
            }

            actionEntities[i].name = cJSON_GetObjectItemCaseSensitive(actionEntity, "name")->valuestring;

            cJSON *value = cJSON_GetObjectItemCaseSensitive(actionEntity, "value");

            if (cJSON_IsString(value)) {
                actionEntities[i].valueType = VEIDES_ENTITY_VALUE_STRING;
                actionEntities[i].valueString = value->valuestring;
            } else if (cJSON_IsBool(value)) {
                actionEntities[i].valueType = VEIDES_ENTITY_VALUE_BOOL;
                actionEntities[i].valueBool = cJSON_IsTrue(value);
            } else if (cJSON_IsNumber(value)) {
                if ((double) value->valueint != value->valuedouble) {
                    actionEntities[i].valueType = VEIDES_ENTITY_VALUE_DOUBLE;
                    actionEntities[i].valueDouble = value->valuedouble;
                } else {
                    actionEntities[i].valueType = VEIDES_ENTITY_VALUE_INT;
                    actionEntities[i].valueInt = value->valueint;
                }
            }
        }
    }

    VeidesClient *veidesClient = (VeidesClient *) client;

    VeidesActionHandler *handler = veides_client_getActionHandler(veidesClient->actionHandlers, name);

    if (handler != NULL) {
        VeidesActionCallbackHandler callback = (VeidesActionCallbackHandler) handler->callback;
        if (callback != NULL) {
            (*callback)(client, name, actionEntities, size);
            veides_utils_freePtr(actionEntities);
            goto endActionReceived;
        }
    }

    handler = veides_client_getAnyActionHandler(veidesClient->actionHandlers);

    if (handler != NULL) {
        VeidesAnyActionCallbackHandler callback = (VeidesAnyActionCallbackHandler) handler->callback;

        if (callback != NULL) {
            (*callback)(client, name, actionEntities, size);
            veides_utils_freePtr(actionEntities);
        }
    }

endActionReceived:
    if (json) {
        cJSON_Delete(json);
    }

    if (pl) {
        free(pl);
    }
}

static void veides_client_methodMessageReceived(VeidesAgentClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen) {
    char *pl = NULL;

    pl = (char *) malloc(payloadlen+1);

    memset(pl, 0, payloadlen+1);
    strncpy(pl, payload, payloadlen);

    char *name = strrchr(topic, '/');
    memmove(name, name+1, strlen(name));

    VEIDES_LOG_DEBUG("Agent method invoked (name=%s,payload=%s)", name, pl);

    VeidesClient *veidesClient = (VeidesClient *) client;

    VeidesMethodHandler *handler = veides_client_getMethodHandler(veidesClient->methodHandlers, name);

    if (handler != NULL) {
        VeidesMethodCallbackHandler callback = (VeidesMethodCallbackHandler) handler->callback;
        if (callback != NULL) {
            (*callback)(client, name, payload, payloadlen);
            goto endMethodInvoked;
        }
    }

    handler = veides_client_getAnyMethodHandler(veidesClient->methodHandlers);

    if (handler != NULL) {
        VeidesAnyMethodCallbackHandler callback = (VeidesAnyMethodCallbackHandler) handler->callback;

        if (callback != NULL) {
            (*callback)(client, name, payload, payloadlen);
        }
    }

endMethodInvoked:
    if (pl) {
        free(pl);
    }
}
