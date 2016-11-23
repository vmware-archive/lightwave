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
 * Checks first two bits of octect.  If they are set, then the octect is a
 * flag specifing a compressed domain name offset.
 */
#ifndef VMDNS_COMPRESSED_NAME
#define VMDNS_COMPRESSED_NAME(Data)     \
    (((Data) & (1 << 0x07))) &&          \
    (((Data) & (1 << 0x06)))
#endif /* VMDNS_COMPRESSED_NAME */

