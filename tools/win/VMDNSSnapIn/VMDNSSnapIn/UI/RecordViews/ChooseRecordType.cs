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
using System.Threading.Tasks;
using System.Windows.Forms;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.UI.RecordViews
{
    partial class ChooseRecordType : Form
    {
        public string ZoneNameString { get; set; }
        public string RecordType { get; set; }

        public ChooseRecordType(string zoneName)
        {
            ZoneNameString = zoneName;
            InitializeComponent();
            this.ZoneNameText.Text = ZoneNameString;
            RecordTypeCombo.SelectedIndex = 0;
        }

        private void DoValidateControls()
        {
            if (String.IsNullOrWhiteSpace(ZoneNameText.Text) || String.IsNullOrWhiteSpace(RecordTypeCombo.Text))
                throw new Exception(MMCUIConstants.VALUES_EMPTY);
        }

        private void OnOKButton(object sender, EventArgs e)
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                RecordType = this.RecordTypeCombo.Text;
                this.Close();
                this.DialogResult = DialogResult.OK;
            });
        }

        private void OnCloseButton(object sender, EventArgs e)
        {
            this.Close();
            this.DialogResult = DialogResult.Cancel;
        }
    }
}
