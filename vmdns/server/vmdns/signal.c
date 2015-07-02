/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

/*
 * Module Name: main
 *
 * Filename: signal.c
 *
 * Abstract: VMware Domain Name Service.
 *
 * Signal handling
 *
 */

#include "includes.h"

#ifndef _WIN32

static
VOID
VmDnsInterruptHandler(
    int Signal
    );

VOID
VmDnsBlockSelectedSignals(
    VOID
    )
{
    sigset_t default_signal_mask;
    sigset_t old_signal_mask;

    sigemptyset(&default_signal_mask);
    sigaddset(&default_signal_mask, SIGINT);
    sigaddset(&default_signal_mask, SIGTERM);
    sigaddset(&default_signal_mask, SIGHUP);
    sigaddset(&default_signal_mask, SIGQUIT);
    sigaddset(&default_signal_mask, SIGPIPE);

    pthread_sigmask(SIG_BLOCK,  &default_signal_mask, &old_signal_mask);
}


DWORD
VmDnsHandleSignals(
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
    action.sa_handler = VmDnsInterruptHandler;

    sysRet = sigaction(SIGINT, &action, NULL);
    dwError = (sysRet != 0) ? errno : 0;
    BAIL_ON_VMDNS_ERROR(dwError);

    // Unblock SIGINT
    sigemptyset(&catch_signal_mask);
    sigaddset(&catch_signal_mask, SIGINT);

    dwError = pthread_sigmask(SIG_UNBLOCK, &catch_signal_mask, NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

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
                VmDnsLog( VMDNS_LOG_LEVEL_INFO, "Handled SIG[%d]\n",which_signal);
                goto error;
            }
            case SIGPIPE:
            {
                VmDnsLog( VMDNS_LOG_LEVEL_INFO, "Handled SIGPIPE");

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
VmDnsInterruptHandler(
    int Signal
    )
{
    if (Signal == SIGINT) {
        raise(SIGTERM);
    }
}

#endif
