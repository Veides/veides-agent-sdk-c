#include "veides_properties.h"
#include "veides_internal.h"

VEIDES_RC VeidesAgentClientProperties_create(VeidesAgentClientProperties **properties) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
        return rc;
    }
    if (*properties != NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Properties object is already initialized (rc=%d)", rc);
        return rc;
    }

    *properties = (VeidesAgentClientProperties *) calloc(1, sizeof(VeidesAgentClientProperties));
    if (*properties == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    VeidesAgentProperties *agentProperties = (VeidesAgentProperties *) calloc(1, sizeof(VeidesAgentProperties));
    if (agentProperties == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    VeidesConnectionProperties *connectionProperties = (VeidesConnectionProperties *) calloc(1, sizeof(VeidesConnectionProperties));
    if (connectionProperties == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    connectionProperties->port = 8883;

    (*properties)->agentProperties = agentProperties;
    (*properties)->connectionProperties = connectionProperties;

    return rc;
}


VEIDES_RC VeidesAgentClientProperties_destroy(VeidesAgentClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
    }

    if (properties->agentProperties != NULL) {
        VeidesAgentProperties *agentProperties = properties->agentProperties;
        veides_utils_freePtr((void *) agentProperties->key);
        veides_utils_freePtr((void *) agentProperties->secretKey);
    }
    if (properties->connectionProperties != NULL) {
        VeidesConnectionProperties *connectionProperties = properties->connectionProperties;
        veides_utils_freePtr((void *) connectionProperties->host);
    }

    veides_utils_freePtr(properties);

    return rc;
}


VEIDES_RC VeidesAgentClientProperties_setProperty(VeidesAgentClientProperties *properties, const char *name, const char *value) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
        return VEIDES_RC_INVALID_HANDLE;
    }

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        goto setPropertyEnd;
    }

    if (!value || *value == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        goto setPropertyEnd;
    }

    if (strcmp(name, "agent.client.id") == 0) {
        if (properties->agentProperties->clientId)
            veides_utils_freePtr((void *) properties->agentProperties->clientId);
        properties->agentProperties->clientId = strdup(value);

        rc = VEIDES_RC_SUCCESS;
    } else if (strcmp(name, "agent.key") == 0) {
        if (properties->agentProperties->key)
            veides_utils_freePtr((void *) properties->agentProperties->key);
        properties->agentProperties->key = strdup(value);

        rc = VEIDES_RC_SUCCESS;
    } else if (strcmp(name, "agent.secret.key") == 0) {
        if (properties->agentProperties->secretKey)
            veides_utils_freePtr((void *) properties->agentProperties->secretKey);
        properties->agentProperties->secretKey = strdup(value);

        rc = VEIDES_RC_SUCCESS;
    } else if (strcmp(name, "client.host") == 0) {
        if (properties->connectionProperties->host)
            veides_utils_freePtr((void *) properties->connectionProperties->host);
        properties->connectionProperties->host = strdup(value);

        rc = VEIDES_RC_SUCCESS;
    } else {
        rc = VEIDES_RC_INVALID_PROPERTY;
    }

setPropertyEnd:
    if (rc == VEIDES_RC_INVALID_PROPERTY) {
        VEIDES_LOG_WARNING("Property %s could not be recognized", name);
    } else if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Invalid value for property %s provided (rc=%d)", name, rc);
    } else {
        VEIDES_LOG_DEBUG("Property %s set to %s", name, value);
    }

    return rc;
}

VEIDES_RC VeidesAgentClientProperties_setPropertiesFromEnv(VeidesAgentClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    char *env = *environ;
    int index = 1;
    int invalidProperties = 0;
    int prefixlen = 7;

    if (!properties) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
        return rc;
    }

    VEIDES_LOG_INFO("Setting properties from environment variables");

    for (; env; index++)
    {
        char *envval = strdup(env);
        char *prop = strtok(envval, "=");
        char *value = strtok(NULL, "=");

        if (prop && *prop != '\0' && !strncasecmp(prop, "VEIDES_", prefixlen) && value && *value != '\0')
        {
            char *name = NULL;
            char *p = NULL;

            p = prop;
            for (; *p; ++p) {
                if (*p == '_') {
                    *p = '.';
                } else {
                    *p = tolower(*p);
                }
            }

            name = prop + prefixlen;
            VEIDES_RC setRc = VeidesAgentClientProperties_setProperty(properties, name, value);

            if (setRc == VEIDES_RC_INVALID_PROPERTY) {
                VEIDES_LOG_WARNING("Environment variable %s could not be recognized", prop);
            } else if (setRc != VEIDES_RC_SUCCESS) {
                invalidProperties++;
                VEIDES_LOG_ERROR("Invalid value for environment variable %s provided (rc=%d)", prop, setRc);
            }
        }

        veides_utils_freePtr((void *) envval);

        env = *(environ + index);
    }

    if (invalidProperties > 0) {
        VEIDES_LOG_WARNING("Found %d INVALID Veides properties", invalidProperties);
    }

    return rc;
}
