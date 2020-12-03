#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "veides_agent_client.h"

#define MAX_LOG_BUFSIZE 8192

volatile int finish = 0;

void log_message(void *target, const char *items, ...) {
    va_list args;

    char buffer[MAX_LOG_BUFSIZE];
    memset(buffer, '0', MAX_LOG_BUFSIZE);

    va_start(args, items);
    vsnprintf(buffer, MAX_LOG_BUFSIZE, items, args);
    va_end(args);

    fprintf(stdout, "Logging %s", buffer);
    fflush(stdout);

    fprintf(target, "%s\n", buffer);
    fflush(target);
}

void sdkLogCallback(int level, char *message) {
    fprintf(stdout, "%s", message);
    fflush(stdout);
}

void sigHandler(int signalNumber) {
    signal(SIGINT, NULL);
    log_message(stdout, "INFO: Received signal: %d", signalNumber);
    finish = 1;
}

void printHelp() {
    printf(
        "Usage: agentBasic [-h] -i <client_id> -k <key> -s <secret_key> -H <host>\n\n"

        "Basic example of connecting agent to Veides\n\n"

        "Options:\n"
        "  -i <client_id>    Client id of agent\n"
        "  -k <key>          Key of agent\n"
        "  -s <secret_key>   Secret key of agent\n"
        "  -H <host>         Host to connect to\n\n"

        "  -h, --help        Displays this help.\n"
    );
}

int sendInitialFacts(VeidesAgentClient *agent) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    // Create facts object
    VeidesFacts *facts = NULL;
    rc = VeidesAgentClientFacts_create(&facts);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize Veides Facts (rc=%d)", rc);
        return rc;
    }

    // Add initial fact
    rc = VeidesAgentClientFacts_addFact(facts, "battery_level", "full");
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to add fact to facts array (rc=%d)", rc);
        return rc;
    }

    rc = VeidesAgentClientFacts_addFact(facts, "charging", "no");
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to add fact to facts array (rc=%d)", rc);
        return rc;
    }

    // Send initial facts
    rc = VeidesAgentClient_sendFacts(agent, facts);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to send facts (rc=%d)", rc);
        return rc;
    }

    // Clear facts array
    VeidesAgentClientFacts_destroy(facts);

    return rc;
}

void printActionEntities(VeidesActionEntity *entities, size_t entitieslen) {
    for (int i = 0; i < entitieslen; i++) {
        if (entities[i].valueType == VEIDES_ENTITY_VALUE_STRING) {
            log_message(stdout, "INFO: %d entity is: name=%s, value=%s", i, entities[i].name, entities[i].valueString);
        } else if (entities[i].valueType == VEIDES_ENTITY_VALUE_INT) {
            log_message(stdout, "INFO: %d entity is: name=%s, value=%d", i, entities[i].name, entities[i].valueInt);
        } else if (entities[i].valueType == VEIDES_ENTITY_VALUE_DOUBLE) {
            log_message(stdout, "INFO: %d entity is: name=%s, value=%f", i, entities[i].name, entities[i].valueDouble);
        } else if (entities[i].valueType == VEIDES_ENTITY_VALUE_BOOL) {
            log_message(stdout, "INFO: %d entity is: name=%s, value=%d", i, entities[i].name, entities[i].valueBool);
        }
    }
}

// void onAnyAction(VeidesAgentClient *agent, char *name, VeidesActionEntity *entities, size_t entitieslen) {
//     log_message(stdout, "INFO: Action received: %s", name ? name : "NULL");

//     printActionEntities(entities, entitieslen);

//     VeidesAgentClient_sendActionCompleted(agent, name);
// }

void onSetLowSpeedAction(VeidesAgentClient *agent, VeidesActionEntity *entities, size_t entitieslen) {
    log_message(stdout, "INFO: Action received: %s", "set_low_speed");

    printActionEntities(entities, entitieslen);

    VeidesAgentClient_sendTrail(agent, "speed_level", "low");

    VeidesAgentClient_sendActionCompleted(agent, "set_low_speed");
}

int main(int argc, char *argv[]) {
    char *host = NULL;
    char *clientId = NULL;
    char *key = NULL;
    char *secretKey = NULL;

    for (int optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
        case 'i': clientId = argv[optind + 1]; optind++; break;
        case 'k': key = argv[optind + 1]; optind++; break;
        case 's': secretKey = argv[optind + 1]; optind++; break;
        case 'H': host = argv[optind + 1]; optind++; break;
        case 'h': printHelp(); exit(0);
        default:
            printHelp();
            exit(0);
        }
    }

    int rc = 0;

    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // Set SDK log handler
    rc = VeidesAgentClient_setLogHandler(&sdkLogCallback);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to set SDK log handler (rc=%d)", rc);
    }

    // Set DEBUG level to see received and sent data. Level is INFO by default
    VeidesAgentClient_setLogLevel(VEIDES_LOG_DEBUG);

    rc = VeidesAgentClientProperties_create(&properties);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize properties (rc=%d)", rc);
        exit(1);
    }

    // Set required properties
    VeidesAgentClientProperties_setProperty(properties, "client.host", host);
    VeidesAgentClientProperties_setProperty(properties, "agent.client.id", clientId);
    VeidesAgentClientProperties_setProperty(properties, "agent.key", key);
    VeidesAgentClientProperties_setProperty(properties, "agent.secret.key", secretKey);

    // Properties may also be provided in environment variables
    // VeidesAgentClientProperties_setPropertiesFromEnv(properties);

    rc = VeidesAgentClient_create(&agent, properties);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize Veides Agent client (rc=%d)", rc);
        exit(1);
    }

    rc = VeidesAgentClient_connect(agent);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to connect to Veides (rc=%d)", rc);
        exit(1);
    }

    // Set a handler for particular action
    VeidesAgentClient_setActionHandler(agent, "set_low_speed", onSetLowSpeedAction);

    // You can also set a handler for any action received.
    // It will execute when there's no callback set for the particular action
    // VeidesAgentClient_setAnyActionHandler(agent, onAnyAction);

    // Send initial facts
    rc = sendInitialFacts(agent);
    if (rc != VEIDES_RC_SUCCESS) {
        exit(1);
    }

    // Send a trail
    rc = VeidesAgentClient_sendTrail(agent, "speed_level", "normal");
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to send trail (rc=%d)", rc);
    }

    // Send ready event
    rc = VeidesAgentClient_sendEvent(agent, "ready_to_rock");
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to send an event (rc=%d)", rc);
    }

    int uptime = 0;

    while (!finish) {
        sleep(1);
        uptime++;

        char trailValue[100];
        sprintf(trailValue, "%d", uptime);

        // Send trail
        rc = VeidesAgentClient_sendTrail(agent, "uptime", trailValue);
        if (rc != VEIDES_RC_SUCCESS) {
            log_message(stderr, "WARN: Failed to send trail (rc=%d)", rc);
        }

        if (uptime == 10) {
            VeidesFacts *facts = NULL;

            VeidesAgentClientFacts_create(&facts);
            VeidesAgentClientFacts_addFact(facts, "battery_level", "low");
            // Send facts update
            rc = VeidesAgentClient_sendFacts(agent, facts);
            if (rc != VEIDES_RC_SUCCESS) {
                log_message(stderr, "WARN: Failed to send facts (rc=%d)", rc);
            }
            VeidesAgentClientFacts_destroy(facts);
        }
    }

    VeidesFacts *facts = NULL;

    rc = VeidesAgentClientFacts_create(&facts);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to initialize Veides Facts (rc=%d)", rc);
    }

    rc = VeidesAgentClientFacts_addFact(facts, "battery_level", "unknown");
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to add fact to facts array (rc=%d)", rc);
    }

    // Send facts update
    rc = VeidesAgentClient_sendFacts(agent, facts);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to send facts (rc=%d)", rc);
    }

    VeidesAgentClientFacts_destroy(facts);

    rc = VeidesAgentClient_disconnect(agent);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to disconnect from Veides (rc=%d)", rc);
        exit(1);
    }

    // Clear Veides Agent client
    VeidesAgentClient_destroy(agent);

    // Clear Veides properties
    VeidesAgentClientProperties_destroy(properties);

    return 0;

}

