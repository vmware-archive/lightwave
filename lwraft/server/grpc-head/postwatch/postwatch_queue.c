/*
 * Mock POST Watch test driver, which simulates integration with
 * postd with pending WatchCreateRequests(). This drives the gRPC
 * server-side logic. Once completed, this test driver module
 * should be removed or conditionally compiled out.
 */


#include "includes.h"

static void *grpc_stream; // This should be a watch_id:stream table

static
void *fake_queue_thread(void *ctx)
{
    postwatch_response_t *response = NULL;
    ppostwatch_event_t event = NULL;
    postwatch_object_t objects[2] = {0};
    postwatch_attribute_t attrib_event1[3] = {0};
    postwatch_attribute_t attrib_event2[2] = {0};
    char *values_event1attr1[3] = {0};
    char *values_event1attr2[2] = {0};
    char *values_event1attr3[1] = {0};
    char *values_event2attr1[2] = {0};
    char *values_event2attr2[1] = {0};
    int i = 10;
    int events_count = 2;

    /*
     * Format of the returned data:
     * 2 Events, event[0] = attributes[3], a[0]=v[3]; a[1]=v[2]; a[2]=v[1]
     * 2 Events, event[1] = attributes[2], a[0]=v[2]; a[1]=v[1];
     * a[0] = element of attribute array
     * v[n] = number of values per attribute
     */

    size_t response_size = sizeof(postwatch_response_t) +
                               (sizeof(postwatch_event_t) * (unsigned) events_count);

    /* Make response */
    response = (ppostwatch_response_t) calloc(1, response_size);
    if (!response)
    {
        return NULL;
    }

    response->watch_id = 31415;
    response->events_count = events_count;

    // Event [0]
    event = &response->events[0];
    event->revision = 3;
    event->event_type = PWE_MOD;
    event->current_object = objects;
    objects[0].version = 5;
    objects[0].dn = "cn=adamhost,cn=Computers,dn=lw_dom,dn=local";
    objects[0].attribute_count = 3;
    objects[0].attribute_list = attrib_event1;
    // Attribute 1: supportedSASLMechanisms
    attrib_event1[0].name = "supportedSASLMechanisms";
    attrib_event1[0].value_count = 3;
    attrib_event1[0].values = values_event1attr1;
    values_event1attr1[0] = "GSS-SPNEGO";
    values_event1attr1[1] = "GSSAPI";
    values_event1attr1[2] = "SRP";
    // Attribute 2: supportedLDAPVersion
    attrib_event1[1].name = "supportedLDAPVersion";
    attrib_event1[1].value_count = 2;
    attrib_event1[1].values = values_event1attr2;
    values_event1attr2[0] = "2";
    values_event1attr2[1] = "3";
    // Attribute 3: rootNamingContext
    attrib_event1[2].name = "rootNamingContext";
    attrib_event1[2].value_count = 1;
    attrib_event1[2].values = values_event1attr3;
    values_event1attr3[0] = "dc=lw_dom,dc=local";

    // Event [1]
    event = &response->events[1];
    event->revision = 4;
    event->event_type = PWE_ADD;
    event->current_object = &objects[1];
    objects[1].version = 1;
    objects[1].dn = "cn=aws-123412,cn=Computers,dn=lw_dom,dn=local";
    objects[1].attribute_count = 2;
    objects[1].attribute_list = attrib_event2;
    // Attribute 1: supportedCapabilities
    attrib_event2[0].name = "supportedCapabilities";
    attrib_event2[0].value_count = 2;
    attrib_event2[0].values = values_event2attr1;
    values_event2attr1[0] = "1.2.840.113556.1.4.800";
    values_event2attr1[1] = "1.2.840.113556.1.4.1935";
    // Attribute 2: dnsHostName
    attrib_event2[1].name = "dnsHostName";
    attrib_event2[1].value_count = 1;
    attrib_event2[1].values = values_event2attr2;
    values_event2attr2[0] = "adam-mint-18.lw_dom.local";

    sleep(2);
    while (i--)
    {
        printf("fake_queue_thread: action here...\n");
        WatchSendEvents(grpc_stream, response);
        sleep(5);
    }
    return NULL;
}

static
int MyCreateRequestCallback(watch_create_request_t *ctx)
{
    printf("MyCreateRequest: <-b %s %s> prev_value=%d start_revision=%ld\n",
           ctx->subtree,
           ctx->filter,
           ctx->prev_value,
           ctx->start_revision);

    /* This is saved by core queue logic with the watch_id */
    grpc_stream = ctx->stream_handle;

    // This is a "fake" test watch_id
    ctx->watch_id = time(NULL) % 65535;

#if 1
{
    pthread_t thr;
    int sts = 0;
    sts = pthread_create(&thr, NULL, fake_queue_thread, NULL);
    if (sts == 0)
    {
        pthread_detach(thr);
    }
}
#endif

    return 0;
}


static
int MyCancelRequestCallback(int64_t watch_id)
{
    int sts = 0;

    printf("MyCancelRequest: %ld\n", watch_id);

    return sts;
}



/* The method PostWatchGetCallbacks() MUST BE implemented here */
postwatch_callbacks_t *PostWatchGetCallbacks(void)
{
    /* This structure must be initialized */
    static
    postwatch_callbacks_t postwatch_queue_callbacks =
    {
        MyCreateRequestCallback,
        MyCancelRequestCallback,
    };
    return &postwatch_queue_callbacks;
}
