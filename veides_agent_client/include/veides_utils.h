#if !defined(VEIDES_UTILS_H)
#define VEIDES_UTILS_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <unistd.h>

#include "veides_rc.h"

extern char **environ;

typedef void *VeidesAgentClient;

typedef enum {
    VEIDES_ENTITY_VALUE_STRING = 0,
    VEIDES_ENTITY_VALUE_BOOL = 1,
    VEIDES_ENTITY_VALUE_DOUBLE = 2,
    VEIDES_ENTITY_VALUE_INT = 3,
} VEIDES_ENTITY_VALUE_TYPE;

typedef struct {
    char *name;
    int valueType;

    char *valueString;
    double valueDouble;
    int valueInt;
    int valueBool;
} VeidesActionEntity;

typedef struct {
    char *name;
    char *value;
} VeidesFact;

typedef struct {
    VeidesFact **entries;
    int count;
} VeidesFacts;

typedef void (*VeidesCallbackHandler)(void *client, char *topic, size_t topiclen, void *payload, size_t payloadlen);

typedef void (*VeidesAnyActionCallbackHandler)(VeidesAgentClient *client, char *name, VeidesActionEntity *entities, size_t entitieslen);

typedef void (*VeidesActionCallbackHandler)(VeidesAgentClient *client, char *name, VeidesActionEntity *entities, size_t entitieslen);

typedef void (*VeidesAnyMethodCallbackHandler)(VeidesAgentClient *client, char *name, char *payload, size_t payloadlen);

typedef void (*VeidesMethodCallbackHandler)(VeidesAgentClient *client, char *name, char *payload, size_t payloadlen);

void veides_utils_freePtr(void *p);
void veides_utils_sleep(long milsecs);
void veides_utils_writeClientVersion(void);
VEIDES_RC veides_utils_topic_match(const char *sub, const char *topic);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_UTILS_H */
