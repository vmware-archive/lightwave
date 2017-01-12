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



#include "includes.h"

int
VmDirPerformUnbind(
   PVDIR_OPERATION pOperation
   )
{
   VmDirLog( LDAP_DEBUG_TRACE, "PerformUnbind: Begin" );

   /*
    * Parse the unbind request.  It looks like this:
    *
    *   UnBindRequest ::= NULL
    */

   assert(pOperation && pOperation->conn);

   VmDirFreeAccessInfo(&(pOperation->conn)->AccessInfo);

   VmDirLog( LDAP_DEBUG_TRACE, "PerformUnbind: End" );

   return LDAP_NOTICE_OF_DISCONNECT;
}

