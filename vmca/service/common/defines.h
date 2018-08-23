/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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

// VMCA Policy

#define VMCA_POLICY_FILE_PATH               VMCA_CONFIG_DIR "/vmca-policy.json"
#define VMCA_POLICY_REQ_UPN_DN              "req.upn.dn"
#define VMCA_POLICY_COND_BEGINS             "begins"
#define VMCA_POLICY_REQ_UPN_RDN             "req.upn.dn.rdn"
#define VMCA_POLICY_REQ_CSR_SUBJ_ORGS       "req.csr.subj.o"
#define VMCA_POLICY_MA_NAME                 "req.upn.cn"
#define VMCA_POLICY_MA_FQDN                 "req.upn.fqdn"
#define VMCA_POLICY_HOSTNAME_TEMPLATE       "<<hostname>>"

#define VMCA_POLICY_NUM                     1
#define VMCA_POLICY_SN_NAME                 "SNValidate"

// RFC 5735 defined private and local networks
#define VMCA_PRIV_NET1                      0xC0A80000      // 192.168.0.0/16
#define VMCA_PRIV_NET2                      0xAC100000      // 172.16.0.0/12
#define VMCA_PRIV_NET3                      0x0A000000      // 10.0.0.0/8
#define VMCA_LOCAL_NET                      0x7F000000      // 127.0.0.0/8
#define VMCA_NETMASK_16                     0xFFFF0000      // 255.255.0.0  (/16 CIDR, class B)
#define VMCA_NETMASK_12                     0xFFF00000      // 255.240.0.0  (/12 CIDR, 16 class B's)
#define VMCA_NETMASK_8                      0xFF000000      // 255.0.0.0    (/8  CIDR, class A)

// Determines if an IP is within a network.  Assumes all values are in hex form.
#define VMCA_IS_IP_IN_NETWORK(ip, net, mask)                            \
    ((((ip) >= ((net) & (mask))) && ((ip) <= (((net) & (mask)) | ~(mask)))) ? 1 : 0)

// Determines if an IP is within RFC defined private networks
#define VMCA_IS_IP_IN_PRIVATE_NETWORK(ip)                               \
    (VMCA_IS_IP_IN_NETWORK((ip), VMCA_PRIV_NET1, VMCA_NETMASK_16) ||    \
     VMCA_IS_IP_IN_NETWORK((ip), VMCA_PRIV_NET2, VMCA_NETMASK_12) ||    \
     VMCA_IS_IP_IN_NETWORK((ip), VMCA_PRIV_NET3, VMCA_NETMASK_8))

#define VMCA_IS_IP_IN_LOCAL_NETWORK(ip)                                 \
    (VMCA_IS_IP_IN_NETWORK((ip), VMCA_LOCAL_NET, VMCA_NETMASK_8))

typedef enum _VMCA_OPENSSL_NID_TYPES
{
    VMCA_OPENSSL_NID_O = 0,
    VMCA_OPENSSL_NID_CN
} VMCA_OPENSSL_NID_TYPES;
