#include "test_cases.h"

#define MAX_THREADS 25
#define MAX_ITERATIONS 100

PSSO_CLIENT_CURL_INIT_CTX pCurlInitCtx = NULL;

static
void
TestClientLockCallBack(
    int mode,
    int         lockNum,
    const char *file,
    int         line)
{
    SSOClientSSLLock(mode, &pCurlInitCtx->pMutexBuffer[lockNum]);
}

static
void*
ThreadWorker(
    void* pData
    )
{
    SSOERROR e = SSOERROR_NONE;
    unsigned int dwIndex = 0;

    for (; dwIndex < MAX_ITERATIONS; ++dwIndex)
    {
          unsigned char buffer[32];

          int rc = RAND_pseudo_bytes(buffer, sizeof(buffer));
          e = ERR_get_error();
          TEST_ASSERT_SUCCESS(e);

          if(rc != 0 && rc != 1) {
            e = SSOERROR_CURL_INIT_FAILURE;
            TEST_ASSERT_SUCCESS(e);
          }
    }

    return NULL;
}

bool
TestSSLRandomGenerator()
{
    SSOERROR e = SSOERROR_NONE;
    pthread_t threads[MAX_THREADS];

    unsigned int dwIndex = 0;

    e = SSOHttpClientGlobalInit(TestClientLockCallBack, &pCurlInitCtx);
    TEST_ASSERT_SUCCESS(e);

    for (; dwIndex < MAX_THREADS; ++dwIndex)
    {
        e = pthread_create(&threads[dwIndex], NULL, ThreadWorker, NULL);
        TEST_ASSERT_SUCCESS(e);
    }

    for (dwIndex = 0; dwIndex < MAX_THREADS; ++dwIndex)
    {
        pthread_join(threads[dwIndex], NULL);
    }

    if (pCurlInitCtx)
    {
        SSOHttpClientGlobalCleanup(pCurlInitCtx);
    }
    return 1;
}
