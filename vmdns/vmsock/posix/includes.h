/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include <config.h>

#include <vmdnssys.h>

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif

#include <vmdns.h>
#include <vmdnsdefines.h>

#include <vmdnscommon.h>
#include <vmsock.h>
#include <vmsockapi.h>
#include <vmsockposix.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
