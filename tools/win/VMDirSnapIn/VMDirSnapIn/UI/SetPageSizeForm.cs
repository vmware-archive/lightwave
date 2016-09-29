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
using System.Linq;
using System.Windows.Forms;
using VMDir.Common;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class SetPageSizeForm : Form
    {
        public int PageSize;
        public SetPageSizeForm(int pageSize)
        {
            this.PageSize = pageSize;
            InitializeComponent();
            this.textBoxPageSize.Text = pageSize.ToString();
        }

        private void buttonSubmit_Click(object sender, EventArgs e)
        {
            if (!ValidateInput())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            PageSize = int.Parse(textBoxPageSize.Text);

            this.Close();
        }

        private bool ValidateInput()
        {
            if (string.IsNullOrWhiteSpace(textBoxPageSize.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_PAGE_SIZE);
                return false;
            }
            try
            {
                var pagesize = int.Parse(textBoxPageSize.Text);
                if (pagesize <= 0)
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_PAGE_SIZE_MINVAL);
                    return false;
                }
                if (pagesize > VMDirConstants.DEFAULT_PAGE_SIZE * 10)
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_PAGE_SIZE_MAXVAL);
                    return false;
                }
            }
            catch (Exception)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_INT_VAL);
                return false;
            }
            return true;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
