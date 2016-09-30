/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

using System;
using VmIdentity.UI.Common;
using VMDir.Common.DTO;
using System.Threading.Tasks;
using VMDir.Common.VMDirUtilities;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaServerNode:ScopeNodeBase
    {

        public bool IsLoggedIn { get; set; }

        private int ret = 0;

        public VMDirServerDTO ServerDTO{ get; set; }

        public VMDirSchemaServerNode(VMDirServerDTO serverDTO)
        {
            this.ServerDTO = serverDTO;
            IsLoggedIn = false;
        }

        public async Task DoLogin()
        {
            Task t = new Task(ServerConnect);
            t.Start();
            if (await Task.WhenAny(t, Task.Delay(VMIdentityConstants.LOGIN_TIMEOUT_IN_MILLI)) == t)
            {
                if (ret == 1)
                    IsLoggedIn = true;
            }
            else
            { 
                throw new Exception(VMIdentityConstants.SERVER_TIMED_OUT);
            }
        }

        public async void ServerConnect()
        {
            try
            {
                ServerDTO.Connection = new LdapConnectionService(ServerDTO.Server, ServerDTO.BindDN, ServerDTO.Password);
                ret = ServerDTO.Connection.CreateConnection();
            }
            catch (Exception e)
            {
                ret = 0;
            }
        }
    }
}