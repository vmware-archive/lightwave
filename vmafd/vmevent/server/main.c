/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
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
