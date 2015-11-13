/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : includes.h
 *
 * Abstract :
 *
 */
#ifndef _WIN32

#include <config.h>

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include <io.h>
#include <tchar.h>
#include <winsock2.h>

#include "banned.h"
#include <pthread.h>
#include <openssl/x509.h>
#endif

#include <vmafdsys.h>

#include <vmdirerrors.h>
#include <vmdirclient.h>
#include <lwsm/lwsm.h>

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_TERM_H
#include <term.h>
#endif

#include <ldap.h>

#include <openssl/x509.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <vmafd.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vmafdcommon.h>
#include <vmafdclient.h>

#include "service.h"
#include "prototypes.h"
