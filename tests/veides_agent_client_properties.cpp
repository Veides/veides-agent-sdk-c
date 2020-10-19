#include "gtest/gtest.h"
#include "veides_agent_client.h"

TEST(veides_agent_client_properties, should_return_error_when_null_handle_provided)
{
    VEIDES_RC rc = VeidesAgentClientProperties_create(NULL);

    EXPECT_EQ(rc, VEIDES_RC_INVALID_HANDLE);
}

TEST(veides_agent_client_properties, should_return_error_when_empty_value_provided)
{
    VeidesAgentClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesAgentClientProperties_setProperty(properties, "client.host", "");

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_agent_client_properties, should_return_error_when_unknown_name_provided)
{
    VeidesAgentClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesAgentClientProperties_setProperty(properties, "unknown.property", "value");

    EXPECT_EQ(rc, VEIDES_RC_INVALID_PROPERTY);
}

TEST(veides_agent_client_properties, should_return_success_when_valid_value_provided)
{
    VeidesAgentClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesAgentClientProperties_setProperty(properties, "client.host", "host");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    EXPECT_EQ(strcmp(properties->connectionProperties->host, "host"), 0);
}

TEST(veides_agent_client_properties, should_return_success)
{
    VeidesAgentClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesAgentClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}
