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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class AboutServerView : Form, IView
    {
        private readonly ServerInfoDto _infoDto;
        public AboutServerView() { }
        public AboutServerView(ServerInfoDto infoDto)
        {
            InitializeComponent();
            _infoDto = infoDto;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {   
             Close();
        }
        public IDataContext DataContext
        {
            get { return _infoDto; }
        }

        private void AboutServerView_Load(object sender, EventArgs e)
        {
            txtProductName.Text = _infoDto.ProductName;
            txtRelease.Text = _infoDto.Release;
            TxtVersion.Text = _infoDto.Version;
        }
    }
}
