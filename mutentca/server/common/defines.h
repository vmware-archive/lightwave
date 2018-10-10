/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

// RFC 5735 defined private and local networks
#define LWCA_PRIV_NET1                      0xC0A80000      // 192.168.0.0/16
#define LWCA_PRIV_NET2                      0xAC100000      // 172.16.0.0/12
#define LWCA_PRIV_NET3                      0x0A000000      // 10.0.0.0/8
#define LWCA_LOCAL_NET                      0x7F000000      // 127.0.0.0/8
#define LWCA_NETMASK_16                     0xFFFF0000      // 255.255.0.0  (/16 CIDR, class B)
#define LWCA_NETMASK_12                     0xFFF00000      // 255.240.0.0  (/12 CIDR, 16 class B's)
#define LWCA_NETMASK_8                      0xFF000000      // 255.0.0.0    (/8  CIDR, class A)

// Determines if an IP is within a network.  Assumes all values are in hex form.
#define LWCA_IS_IP_IN_NETWORK(ip, net, mask)                            \
    ((((ip) >= ((net) & (mask))) && ((ip) <= (((net) & (mask)) | ~(mask)))) ? 1 : 0)

// Determines if an IP is within RFC defined private networks
#define LWCA_IS_IP_IN_PRIVATE_NETWORK(ip)                               \
    (LWCA_IS_IP_IN_NETWORK((ip), LWCA_PRIV_NET1, LWCA_NETMASK_16) ||    \
     LWCA_IS_IP_IN_NETWORK((ip), LWCA_PRIV_NET2, LWCA_NETMASK_12) ||    \
     LWCA_IS_IP_IN_NETWORK((ip), LWCA_PRIV_NET3, LWCA_NETMASK_8))

#define LWCA_IS_IP_IN_LOCAL_NETWORK(ip)                                 \
    (LWCA_IS_IP_IN_NETWORK((ip), LWCA_LOCAL_NET, LWCA_NETMASK_8))

#define LWCA_OIDC_PORT 443
#define LWCA_OIDC_SCOPE "openid offline_access id_groups at_groups rs_post"
