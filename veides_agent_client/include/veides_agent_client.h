#if !defined(VEIDES_AGENT_CLIENT_H)
#define VEIDES_AGENT_CLIENT_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include <MQTTProperties.h>

#include "veides_rc.h"
#include "veides_utils.h"
#include "veides_logger.h"
#include "veides_properties.h"

/**
 * @brief Creates VeidesAgentClient object that is able to interact with Veides. To destroy object use VeidesAgentClient_destroy()
 *
 * @param agent
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_create(VeidesAgentClient **agent, VeidesAgentClientProperties *properties);

/**
 * @brief Destroys VeidesAgentClient object created by VeidesAgentClient_create()
 *
 * @param agent
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_destroy(VeidesAgentClient *agent);

/**
 * @brief Connect Agent to Veides
 *
 * @param agent
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_connect(VeidesAgentClient *agent);

/**
 * @brief Disconnect Agent from Veides
 *
 * @param agent
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_disconnect(VeidesAgentClient *agent);

/**
 * @brief Set a callback for the particular action
 *
 * @param agent
 * @param name
 * @param callback
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_setActionHandler(VeidesAgentClient *agent, char *name, VeidesActionCallbackHandler callback);

/**
 * @brief Set a callback for any action. It will execute when there's no
 * callback set for the particular action (see VeidesAgentClient_setActionHandler())
 *
 * @param agent
 * @param callback
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_setAnyActionHandler(VeidesAgentClient *agent, VeidesAnyActionCallbackHandler callback);

/**
 * @brief Set a callback for the particular method
 *
 * @param agent
 * @param name
 * @param callback
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_setMethodHandler(VeidesAgentClient *agent, char *name, VeidesMethodCallbackHandler callback);

/**
 * @brief Set a callback for any method. It will execute when there's no
 * callback set for the particular method (see VeidesAgentClient_setMethodHandler())
 *
 * @param agent
 * @param callback
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_setAnyMethodHandler(VeidesAgentClient *agent, VeidesAnyMethodCallbackHandler callback);

/**
 * @brief Send the response to invoked method
 *
 * @param agent
 * @param name
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_sendMethodResponse(VeidesAgentClient *agent, char *name, char *payload, int code);

/**
 * @brief Send action completed message
 *
 * @param agent
 * @param name
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_sendActionCompleted(VeidesAgentClient *agent, char *name);

/**
 * @brief Send an event
 *
 * @param agent
 * @param name
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_sendEvent(VeidesAgentClient *agent, char *name);

/**
 * @brief Create VeidesFacts object used with VeidesAgentClient_sendFacts(). To destroy object use VeidesAgentClientFacts_destroy()
 *
 * @param facts
 */
LIBRARY_API VEIDES_RC VeidesAgentClientFacts_create(VeidesFacts **facts);

/**
 * @brief Destroys VeidesFacts array created with VeidesAgentClientFacts_create()
 *
 * @param facts
 */
LIBRARY_API VEIDES_RC VeidesAgentClientFacts_destroy(VeidesFacts *facts);

/**
 * @brief Add a fact to VeidesFacts array
 *
 * @param facts
 * @param name
 * @param value
 */
LIBRARY_API VEIDES_RC VeidesAgentClientFacts_addFact(VeidesFacts *facts, char *name, char *value);

/**
 * @brief Send new fact(s) value(s)
 *
 * @param agent
 * @param name
 * @param value
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_sendFacts(VeidesAgentClient *agent, VeidesFacts *facts);

/**
 * @brief Send a trail
 *
 * @param agent
 * @param name
 * @param value
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_sendTrail(VeidesAgentClient *agent, char *name, char *value);

/**
 * @brief Set SDK log handler
 *
 * @param handler
 */
LIBRARY_API VEIDES_RC VeidesAgentClient_setLogHandler(VeidesLogHandler *handler);

/**
 * @brief Set SDK log level
 *
 * @param level
 */
LIBRARY_API void VeidesAgentClient_setLogLevel(int level);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_AGENT_CLIENT_H */

