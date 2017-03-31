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

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int _tmain(int argc, TCHAR *targv[])
#endif
{
    while (1)
    {
        CHAR pszChoice[16] = { '\0' };
        int choice = -1;

        VmDirReadString(
            "\n\n==================\n"
            "Please select:\n"
            "0. exit\n"
            "1. Test LDAP connectivity\n"
            "2. Force start replication cycle\n"
            "3. Reset account password\n"
            "4. Set log level and mask\n"
            "5. Set vmdir state\n"
            "6. Get vmdir state\n"
            "7. Get vmdir log level and mask\n"
//            "99. Set SRP Authentication data\n"  do not want to expose this to users.  internal test only.
            "==================\n\n",
            pszChoice,
            sizeof(pszChoice),
            FALSE);

        choice = atoi(pszChoice);

        if (!choice)
        {
            goto cleanup;
        }

        switch (choice)
        {
          case 1:
              VdcadminTestSASLClient();
              break;

          case 2:
              VdcadminReplNow();
              break;

          case 3:
              VdcadminForceResetPassword();
              break;

          case 4:
              VdcadminSetLogParameters();
              break;

          case 5:
              VdcadminSetVmdirState();
              break;

          case 6:
              VdcadminGetVmdirState();
              break;

          case 7:
              VdcadminGetLogParameters();
              break;

          case 99:
              VdcadminSetSRPAuthData();
              break;

          default:
              goto cleanup;
        }
    }

cleanup:

    return 0;

}
