/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : defines.h
 *
 * Abstract :
 *
 */

typedef enum
{
    CDC_COMMAND_UNKNOWN = 0,
    CDC_COMMAND_GET_DC_NAME,
    CDC_COMMAND_ENABLE_DEFAULT_HA,
    CDC_COMMAND_ENABLE_LEGACY_HA,
    CDC_COMMAND_GETSTATE_CLIENT_AFFINITY,
    CDC_COMMAND_CACHE_LIST,
    CDC_COMMAND_CACHE_REFRESH
} CDC_COMMAND, *PCDC_COMMAND;

#define BAIL_ON_CDC_CLI_ERROR(dwError, message) \
  if (dwError) \
  { \
      fprintf (stderr, "Error: %s\n", message); \
      goto error; \
  }

