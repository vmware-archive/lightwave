/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#define BAIL_ON_VMSOCK_ERROR(dwError) \
        if (dwError) \
           goto error;

#ifdef WIN32
#define inet_pton(x, y, z) InetPtonA(x, y, z)
#endif
