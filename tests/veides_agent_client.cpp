#include "gtest/gtest.h"
#include "veides_agent_client.h"

TEST(veides_agent_client, should_return_error_when_null_handle_provided)
{
    VeidesAgentClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesAgentClient_create(NULL, properties);

    EXPECT_EQ(rc, VEIDES_RC_INVALID_HANDLE);
}

TEST(veides_agent_client, should_return_error_when_null_properties_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClient_create(&agent, NULL);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client, should_return_error_when_agent_properties_not_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesAgentClientProperties_setProperty(properties, "client.host", "host");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client, should_return_error_when_invalid_client_host_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesAgentClientProperties_setProperty(properties, "agent.client.id", "id");
    VeidesAgentClientProperties_setProperty(properties, "agent.key", "some_key");
    VeidesAgentClientProperties_setProperty(properties, "agent.secret.key", "some_key");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesAgentClientProperties_setProperty(properties, "client.host", "");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client, should_return_error_when_invalid_agent_key_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesAgentClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesAgentClientProperties_setProperty(properties, "agent.client.id", "id");
    VeidesAgentClientProperties_setProperty(properties, "agent.secret.key", "some_key");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesAgentClientProperties_setProperty(properties, "agent.key", "");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client, should_return_error_when_invalid_agent_secret_key_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesAgentClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesAgentClientProperties_setProperty(properties, "agent.client.id", "id");
    VeidesAgentClientProperties_setProperty(properties, "agent.key", "some_key");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesAgentClientProperties_setProperty(properties, "agent.secret.key", "");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client, should_return_error_when_invalid_agent_client_id_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesAgentClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesAgentClientProperties_setProperty(properties, "agent.key", "some_key");
    VeidesAgentClientProperties_setProperty(properties, "agent.secret.key", "some_key");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesAgentClientProperties_setProperty(properties, "agent.client.id", "");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client, should_return_success_when_valid_properties_provided)
{
    VeidesAgentClientProperties *properties = NULL;
    VeidesAgentClient *agent = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesAgentClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesAgentClientProperties_setProperty(properties, "agent.client.id", "id");
    VeidesAgentClientProperties_setProperty(properties, "agent.key", "some_key");
    VeidesAgentClientProperties_setProperty(properties, "agent.secret.key", "some_key");

    rc = VeidesAgentClient_create(&agent, properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}
