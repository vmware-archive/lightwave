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



#define VMW_DEPLOY_LOG_TIME_FORMAT "%Y%m%d%H%M%S"

typedef VOID (*PFN_LOG_CALLBACK)(
                  PVMW_DEPLOY_LOG_CONTEXT pContext,
                  VMW_DEPLOY_LOG_LEVEL    logLevel,
                  PCSTR                   pszFormat,
                  va_list                 args
                  );

#define VMW_DEPLOY_SAFE_LOG_STRING(str) ((str) ? (str) : "")

#define VMW_DEPLOY_LOCK_MUTEX(pMutex, bLocked) \
        if (!(bLocked)) { \
            pthread_mutex_lock(pMutex); \
            (bLocked) = TRUE; \
        }

#define VMW_DEPLOY_UNLOCK_MUTEX(pMutex, bLocked) \
        if (bLocked) { \
            pthread_mutex_unlock(pMutex); \
            (bLocked) = FALSE; \
        }

#define VMW_LDAP_CONNECT_MAX_RETRIES 2
#define VMW_LDAP_CONNECT_RETRY_DELAY_SECS 3

#define VMW_START_VMDIRD "systemctl start vmware-vmdird.service"
#define VMW_START_VMAFDD "systemctl start vmware-vmafdd.service"
#define VMW_START_VMCAD  "systemctl start vmware-vmcad.service"
#define VMW_START_VMDNSD "systemctl start vmware-vmdnsd.service"

#define VMW_STOP_VMDIRD "systemctl stop vmware-vmdird.service"
#define VMW_STOP_VMAFDD "systemctl stop vmware-vmafdd.service"
#define VMW_STOP_VMCAD  "systemctl stop vmware-vmcad.service"
#define VMW_STOP_VMDNSD "systemctl stop vmware-vmdnsd.service"

#define VMW_ENABLE_MDBWAL   "/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/MdbEnableWal 1"
#define VMW_DISABLE_MDBWAL  "/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/MdbEnableWal 0"


