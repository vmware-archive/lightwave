/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System.ComponentModel;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMDirSnapIn.ScopeNodes;
using VMDirSnapIn.Services;
using System.Drawing;
using VMDirSnapIn.UI;
using VMwareMMCIDP.UI.Common.Utilities;
[assembly: PermissionSetAttribute(SecurityAction.RequestMinimum, Unrestricted = true)]
namespace VMDirSnapIn
{
    [RunInstaller(true)]
    public class InstallUtilSupport : SnapInInstaller
    {

        private void InitializeComponent()
        {

        }
    }
    [SnapInSettings("{4262730D-8B69-4581-8E39-264225BA302B}", DisplayName = "Lightwave Directory Browser",
         Description = "Lightwave Directory Browser")]
    public class VMDirSnapIn : SnapIn
    {
        public VMDirSnapIn()
        {
            VMDirEnvironment.Instance.LoadLocalData();
            InitConsole();
        }

        protected override void OnInitialize()
        {
            base.OnInitialize();

            VMDirEnvironment.Instance.SnapIn = this;
            MMCDlgHelper.snapIn = this;
        }
        
        void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
            this.RootNode.Children.Add(node);
        }
        void InitConsole()
        {
            var il = new ImageList();
            this.SmallImages.AddStrip(VMDirEnvironment.Instance.GetToolbarImage());
            this.RootNode = new VMDirRootNode();
        }
        protected override void OnShutdown(AsyncStatus status)
        {
            // saves data to local xml file
            VMDirEnvironment.Instance.SaveLocalData();
        }
    }
}