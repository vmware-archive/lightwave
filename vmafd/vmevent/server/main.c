/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : main.c
 *
 * Abstract :
 *
 */
#include "includes.h"

int
main(
    int   argc,
    const char* argv[]
    )
{
    DWORD dwError = 0;
    const char* pszSmNotify = NULL;
    int notifyFd = -1;
    int notifyCode = 0;
    int ret = -1;

    setlocale(LC_ALL, "");

    EventLogBlockSelectedSignals();

    dwError  = EventLogInitialize();
    BAIL_ON_VMEVENT_ERROR(dwError);
    VMEVENT_LOG_INFO("EventLogService Service starting...");

    // interact with likewise service manager (start/stop control)
    if ((pszSmNotify = getenv("LIKEWISE_SM_NOTIFY")) != NULL)
    {
        notifyFd = atoi(pszSmNotify);

        do
        {
            ret = write(notifyFd, &notifyCode, sizeof(notifyCode));

        } while (ret != sizeof(notifyCode) && errno == EINTR);

        if (ret < 0)
        {
            VMEVENT_LOG_ERROR("Could not notify service manager: %s (%i)",
                            strerror(errno),
                            errno);
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMEVENT_ERROR(dwError);
        }

        close(notifyFd);
    }

    // main thread waits on signals
    dwError = EventLogHandleSignals();
    BAIL_ON_VMEVENT_ERROR(dwError);

    VMEVENT_LOG_INFO("EventLogService Service exiting...");

cleanup:

    EventLogShutdown();

    return (dwError);

error:

    VMEVENT_LOG_ERROR("EventLogService exiting due to error [code:%d]", dwError);

    goto cleanup;
}
