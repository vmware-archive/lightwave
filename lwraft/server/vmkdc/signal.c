/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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



/*
 * Module Name: main
 *
 * Filename: signal.c
 *
 * Abstract: VMware Kdc Service.
 *
 * Signal handling
 *
 */

#include "includes.h"

#ifndef _WIN32

static
VOID
VmKdcInterruptHandler(
    int Signal
    );

VOID
VmKdcBlockSelectedSignals(
    VOID
    )
{
    sigset_t default_signal_mask;

    sigemptyset(&default_signal_mask);
    sigaddset(&default_signal_mask, SIGINT);
    sigaddset(&default_signal_mask, SIGTERM);
    sigaddset(&default_signal_mask, SIGHUP);
    sigaddset(&default_signal_mask, SIGQUIT);
    sigaddset(&default_signal_mask, SIGPIPE);

    pthread_sigmask(SIG_BLOCK,  &default_signal_mask, NULL);
}

DWORD
VmKdcHandleSignals(
    VOID
    )
{
    DWORD dwError = 0;
    struct sigaction action;
    sigset_t catch_signal_mask;
    int which_signal = 0;
    int sysRet = 0;

    // After starting up threads, we now want to handle SIGINT async
    // instead of using sigwait() on it.  The reason for this is so
    // that a debugger (such as gdb) can break in properly.
    // See http://sourceware.org/ml/gdb/2007-03/msg00145.html and
    // http://bugzilla.kernel.org/show_bug.cgi?id=9039.

    memset(&action, 0, sizeof(action));
    action.sa_handler = VmKdcInterruptHandler;

    sysRet = sigaction(SIGINT, &action, NULL);
    dwError = (sysRet != 0) ? errno : 0;
    BAIL_ON_VMKDC_ERROR(dwError);

    // Unblock SIGINT
    sigemptyset(&catch_signal_mask);
    sigaddset(&catch_signal_mask, SIGINT);

    dwError = pthread_sigmask(SIG_UNBLOCK, &catch_signal_mask, NULL);
    BAIL_ON_VMKDC_ERROR(dwError);

    // These should already be blocked...
    sigemptyset(&catch_signal_mask);
    sigaddset(&catch_signal_mask, SIGTERM);
    sigaddset(&catch_signal_mask, SIGQUIT);
    sigaddset(&catch_signal_mask, SIGHUP);
    sigaddset(&catch_signal_mask, SIGPIPE);

    while (1)
    {
        /* Wait for a signal to arrive */
        sigwait(&catch_signal_mask, &which_signal);

        switch (which_signal)
        {
            case SIGINT:
            case SIGQUIT:
            case SIGTERM:
            {
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Handled SIG[%d]\n",which_signal);
                goto error;
            }
            case SIGPIPE:
            {
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Handled SIGPIPE");
                break;
            }
            case SIGHUP:
            {
                break;
            }
            default:
                break;
        }
    }

error:

    return dwError;
}

static
VOID
VmKdcInterruptHandler(
    int Signal
    )
{
    if (Signal == SIGINT) {
        raise(SIGTERM);
    }
}

static
PVOID
VmKdcSignalService(
    PVOID pInfo)
{
    VmKdcHandleSignals();

    VmKdcdStateSet(VMKDC_STOPPING);

    return NULL;
}

DWORD
VmKdcInitSignalThread(
    PVMKDC_GLOBALS pGlobals)
{
    DWORD dwError = 0;
    int   sts = 0;

    sts = pthread_create(
            &pGlobals->thread,
            NULL,
            VmKdcSignalService,
            pGlobals);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}
#endif
