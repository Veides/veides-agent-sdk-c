#include "gtest/gtest.h"
#include "veides_utils.h"

TEST(veides_utils, should_return_success_when_valid_input_provided)
{
    VEIDES_RC rc = veides_utils_topic_match("test/#", "test/");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_utils_topic_match("test/+", "test/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_utils_topic_match("#", "test/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_utils_topic_match("test/+/+/foo", "test/bar/baz/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_utils_topic_match("test/#", "test/bar/baz/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_utils_topic_match("test/+/#", "test/bar/baz/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_utils_topic_match("#", "$SYS/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}

TEST(veides_utils, should_return_error_when_valid_input_provided_but_does_not_match)
{
    VEIDES_RC rc = veides_utils_topic_match("test/+", "test/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match("test/#bar", "test/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match("test/#", "tes/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);
}

TEST(veides_utils, should_return_error_when_empty_input_provided)
{
    VEIDES_RC rc = veides_utils_topic_match(NULL, NULL);

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match(NULL, "");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match("", NULL);

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match(NULL, "test");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match("test", NULL);

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match("test", "");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_utils_topic_match("", "test");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);
}
