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
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMwareMMCIDP.UI;
using VMCA;
using VMCA.Client;
using VMCASnapIn.DTO;
using VMCASnapIn.Utilities;

namespace VMCASnapIn.Nodes
{
    public class VMCAPersonalNode : ChildScopeNode
    {
        public VMCAPersonalNode(VMCAServerDTO dto):base(dto)
        {
            DisplayName = "Personal";
            Tag = -1;

            InitNode();
        }

        void InitNode()
        {
            var keysNode = new VMCAKeyPairNode(ServerDTO);
            this.Children.Add(keysNode);

            var certsNode = new VMCAPersonalCertificatesNode(ServerDTO);
            this.Children.Add(certsNode);

            var csrNode = new VMCACSRNode(ServerDTO);
            this.Children.Add(csrNode);
        }
    }
}
