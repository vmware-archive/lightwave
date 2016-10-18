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



#define BAIL_ON_DEPLOY_ERROR(errcode) \
	if (errcode) {  \
		goto error; \
    }

#define VMW_DEPLOY_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) { \
    VmwDeployFreeMemory(pMemory); \
    }

#define IsNullOrEmptyString(str) (!(str) || !(*str))

#define VMW_DEFAULT_DOMAIN_NAME "vsphere.local"

#define VMW_DIR_SVC_NAME   "vmdir"
#define VMW_VMCA_SVC_NAME  "vmca"
#define VMW_VMAFD_SVC_NAME "vmafd"
#define VMW_VMDNS_SVC_NAME "vmdns"
#define VMW_DCERPC_SVC_NAME "dcerpc"

#define VMCA_RPC_TCP_END_POINT "2014"

#define VMW_TIME_SECS_PER_MINUTE   ( 60)
#define VMW_SECONDS_IN_DAY  (24 * 60 * 60)
#define VMW_SECONDS_IN_YEAR (365 * VMW_SECONDS_IN_DAY)

#define VMW_DEFAULT_CA_PATH "/etc/ssl/certs"
#define VMW_ADMIN_NAME      "Administrator"
#define VMW_DEFAULT_COUNTRY "US"
#define VMW_DEFAULT_CERT_VALIDITY (10 * VMW_SECONDS_IN_YEAR)
#define VMW_DEFAULT_ROOTCERT_PREDATE (3 * VMW_SECONDS_IN_DAY)
#define VMW_CERT_EXPIRY_START_LAG (10 * VMW_TIME_SECS_PER_MINUTE)



#define VMW_KEY_SIZE        2048

#define VMW_VMDIR_SSL_CERT_FILE "vmdircert.pem"
#define VMW_VMDIR_PRIV_KEY_FILE "vmdirkey.pem"

#define VMW_DEFAULT_COUNTRY_CODE "US"

#define VMW_DEPLOY_LOG_MSG(logLevel, format, ...) \
        VmwDeployLogMessage(logLevel, format, ## __VA_ARGS__)
#define VMW_DEPLOY_LOG_ERROR(format, ...)   \
        VMW_DEPLOY_LOG_MSG(VMW_DEPLOY_LOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define VMW_DEPLOY_LOG_WARNING(format, ...) \
        VMW_DEPLOY_LOG_MSG(VMW_DEPLOY_LOG_LEVEL_WARNING, format, ## __VA_ARGS__)
#define VMW_DEPLOY_LOG_INFO(format, ...)    \
        VMW_DEPLOY_LOG_MSG(VMW_DEPLOY_LOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define VMW_DEPLOY_LOG_VERBOSE(format, ...) \
        VMW_DEPLOY_LOG_MSG(VMW_DEPLOY_LOG_LEVEL_VERBOSE, format, ## __VA_ARGS__)
#define VMW_DEPLOY_LOG_DEBUG(format, ...)   \
        VMW_DEPLOY_LOG_MSG(VMW_DEPLOY_LOG_LEVEL_DEBUG, format, ## __VA_ARGS__)

