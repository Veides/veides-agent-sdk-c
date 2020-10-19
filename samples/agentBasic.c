#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>

#include "veides_agent_client.h"

volatile int finish = 0;

void sigHandler(int signalNumber) {
    signal(SIGINT, NULL);
    fprintf(stdout, "Received signal: %d\n", signalNumber);
    finish = 1;
}

void logCallback(int level, char *message) {
    fprintf(stdout, "%s", message);
    fflush(stdout);
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

void printActionEntities(VeidesActionEntity *entities, size_t entitieslen) {
    for (int i = 0; i < entitieslen; i++) {
        if (entities[i].valueType == VEIDES_ENTITY_VALUE_STRING) {
            fprintf(stdout, "%d entity is: name=%s, value=%s\n", i, entities[i].name, entities[i].valueString);
        } else if (entities[i].valueType == VEIDES_ENTITY_VALUE_INT) {
            fprintf(stdout, "%d entity is: name=%s, value=%d\n", i, entities[i].name, entities[i].valueInt);
        } else if (entities[i].valueType == VEIDES_ENTITY_VALUE_DOUBLE) {
            fprintf(stdout, "%d entity is: name=%s, value=%f\n", i, entities[i].name, entities[i].valueDouble);
        } else if (entities[i].valueType == VEIDES_ENTITY_VALUE_BOOL) {
            fprintf(stdout, "%d entity is: name=%s, value=%d\n", i, entities[i].name, entities[i].valueBool);
        }
    }
}

void onAnyAction(VeidesAgentClient *agent, char *name, VeidesActionEntity *entities, size_t entitieslen) {
    fprintf(stdout, "[onAnyAction] Action received: %s\n", name ? name : "NULL");

    printActionEntities(entities, entitieslen);

    fflush(stdout);

    VeidesAgentClient_sendActionCompleted(agent, name);
}

void onAction(VeidesAgentClient *agent, VeidesActionEntity *entities, size_t entitieslen) {
    fprintf(stdout, "[onAction] Action received: %s\n", "set_low_speed");

    printActionEntities(entities, entitieslen);

    fflush(stdout);

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
    rc = VeidesAgentClient_setLogHandler(&logCallback);
    if (rc != VEIDES_RC_SUCCESS) {
        fprintf(stderr, "WARN: Failed to set SDK log handler (rc=%d)\n", rc);
        exit(1);
    }

    // Use VeidesAgentClient_setLogLevel() to change SDk log level
    // VeidesAgentClient_setLogLevel(VEIDES_LOG_DEBUG);

    rc = VeidesAgentClientProperties_create(&properties);
    if (rc != VEIDES_RC_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize properties (rc=%d)\n", rc);
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
        fprintf(stderr, "ERROR: Failed to initialize Veides Agent client (rc=%d)\n", rc);
        exit(1);
    }

    rc = VeidesAgentClient_connect(agent);
    if (rc != VEIDES_RC_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to connect to Veides (rc=%d)\n", rc);
        exit(1);
    }

    // Set a handler for any action received
    // VeidesAgentClient_setAnyActionHandler(agent, onAnyAction);

    // Set a handler for particular action
    // VeidesAgentClient_setActionHandler(agent, "set_low_speed", onAction);

    // Create facts object
    // VeidesFacts *facts = NULL;
    // rc = VeidesAgentClientFacts_create(&facts);
    // if (rc != VEIDES_RC_SUCCESS) {
        // fprintf(stderr, "ERROR: Failed to initialize Veides Facts (rc=%d)\n", rc);
    // }

    // Add initial fact
    // rc = VeidesAgentClientFacts_addFact(facts, "battery_level", "critical");
    // if (rc != VEIDES_RC_SUCCESS) {
        // fprintf(stderr, "ERROR: Failed to add fact to facts array (rc=%d)\n", rc);
    // }

    // Send initial facts
    // rc = VeidesAgentClient_sendFacts(agent, facts);
    // if (rc != VEIDES_RC_SUCCESS) {
        // fprintf(stderr, "ERROR: Failed to send facts (rc=%d)\n", rc);
    // }

    // Clear facts array
    // VeidesAgentClientFacts_destroy(facts);

    // Send ready event
    // rc = VeidesAgentClient_sendEvent(agent, "ready_to_rock");
    // if (rc != VEIDES_RC_SUCCESS) {
        // fprintf(stderr, "ERROR: Failed to send an event (rc=%d)\n", rc);
        // exit(1);
    // }

    int upTime = 0;

    while(!finish) {
        char trailValue[100];
        sprintf(trailValue, "%d", upTime);

        // Send example trail (to see the value in the Console you need to setup dashboard first)
        // rc = VeidesAgentClient_sendTrail(agent, "up_time", trailValue);

        sleep(1);
        upTime++;
    }

    rc = VeidesAgentClient_disconnect(agent);
    if (rc != VEIDES_RC_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to disconnect from Veides (rc=%d)\n", rc);
        exit(1);
    }

    // Clear Veides Agent client
    VeidesAgentClient_destroy(agent);

    // Clear Veides properties
    VeidesAgentClientProperties_destroy(properties);

    return 0;

}

