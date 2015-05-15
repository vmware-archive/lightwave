#ifndef _CONFIG_H
#define _CONFIG_H
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "roken.h"
#include "hex.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#define strtok_r strtok_s
#define snprintf _snprintf
#define strdup _strdup
#endif
#endif
