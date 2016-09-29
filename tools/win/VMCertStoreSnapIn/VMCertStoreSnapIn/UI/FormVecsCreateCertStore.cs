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
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMCertStore.Common.DTO;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.UI
{
    public partial class FormVecsCreateCertStore : Form
    {
        CreateCertStoreDTO _DTO = new CreateCertStoreDTO();
        public CreateCertStoreDTO CertStoreDTO { get { return _DTO; } }

        public FormVecsCreateCertStore()
        {
            InitializeComponent();
        }

        public bool ValidateForm()
        {
            string msg = null;
            if (string.IsNullOrWhiteSpace(textBoxStoreName.Text))
                msg = MMCUIConstants.STORE_ENT;
            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (!ValidateForm())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            _DTO.StoreName = textBoxStoreName.Text;

            this.Close();
        }
    }
}
