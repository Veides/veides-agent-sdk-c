#if !defined(VEIDES_BASE_CLIENT_H)
#define VEIDES_BASE_CLIENT_H

#if defined(__cplusplus)
 extern "C" {
#endif

typedef struct {
    int type;
    char *topic;
    void *callback;
} VeidesHandler;

typedef struct {
    char *name;
    void *callback;
} VeidesActionHandler;

typedef struct {
    VeidesHandler **entries;
    int count;
} VeidesHandlers;

typedef struct {
    VeidesActionHandler **entries;
    int count;
    int anyActionId;
} VeidesActionHandlers;

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_BASE_CLIENT_H */