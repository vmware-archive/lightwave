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
 * Module Name: utils.c
 *
 * Filename: utils.c
 *
 * Abstract:
 *
 * Utility functions
 *
 */

#include "includes.h"

VMCA_DB_CERTIFICATE_STATUS
VMCAMapToDBStatus(CERTIFICATE_STATUS st)
{
	switch(st)
	{
	case CERTIFICATE_ACTIVE : return VMCA_DB_CERTIFICATE_STATUS_ACTIVE;
	case CERTIFICATE_REVOKED : return VMCA_DB_CERTIFICATE_STATUS_REVOKED;
	case CERTIFIFCATE_EXPIRED : return VMCA_DB_CERTIFICATE_STATUS_EXPIRED;
	case CERTIFICATE_ALL : return VMCA_DB_CERTIFICATE_STATUS_ALL;
	}
     return VMCA_DB_CERTIFICATE_STATUS_ALL;
}
