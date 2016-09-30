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
    public partial class AddNewSignatureAlgorithm : Form,IView
    {
        private SignatureAlgorithmDto _dto;
        public AddNewSignatureAlgorithm()
        {
            InitializeComponent();
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            var max = (int)nudMaxKeySize.Value;
            var min = (int)nudMinKeySize.Value;
            if (max < min)
            {
                MMCDlgHelper.ShowError("MAX Key size cannot be less than MIN key size");
            }
            else
            {
                _dto = new SignatureAlgorithmDto { MaxKeySize = max, MinKeySize = min, Priority = (int)nudPriority.Value };
                DialogResult = DialogResult.OK;
                Close();
            }
        }

        public Dto.IDataContext DataContext
        {
            get { return _dto; }
        }
    }
}
