#if !defined(VEIDES_RC_H)
#define VEIDES_RC_H

#if defined(__cplusplus)
 extern "C" {
#endif

typedef enum {
    VEIDES_RC_SUCCESS = 0,
    VEIDES_RC_FAILURE = 1,
    VEIDES_RC_NOMEM = 10001,
    VEIDES_RC_INVALID_HANDLE = 10010,
    VEIDES_RC_INVALID_PROPERTY = 10020,
    VEIDES_RC_NULL_PARAM = 10030,
    VEIDES_RC_INVALID_PARAM_VALUE = 10040,
    VEIDES_RC_NOT_CONNECTED = 10050,
    VEIDES_RC_TIMEOUT = 10060,
    VEIDES_RC_HANDLER_NOT_FOUND = 10070
} VEIDES_RC;

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_RC_H */

