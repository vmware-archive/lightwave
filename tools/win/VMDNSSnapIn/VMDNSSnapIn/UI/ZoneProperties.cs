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
using VMDNS.Common;
using VMDNSSnapIn.Nodes;
using VMwareMMCIDP.UI.Common.Utilities;

/*
 * @author Sumalatha Abhishek
 */

namespace VMDNSSnapIn.UI
{
    partial class ZoneProperties : Form
    {
        private VMDNSZoneEntryNode zoneNode;

        internal ZoneProperties(VMDNSZoneEntryNode zoneNode)
        {
            InitializeComponent();
            this.zoneNode = zoneNode;
            InitialiseUIFieldsFromZoneValues(); 
            
        }

        private void InitialiseUIFieldsFromZoneValues()
        {
            this.PrimaryServerName.Text = zoneNode.CurrentZone.DNSName;
            this.AdminEmail.Text = zoneNode.CurrentZone.AdminEmail;
            this.SerialNumber.Text = zoneNode.CurrentZone.Serial.ToString();
            this.ZoneType.Text = zoneNode.CurrentZone.RefreshInterval.ToString();
            this.RetryInterval.Text = zoneNode.CurrentZone.RetryInterval.ToString();
            this.ZoneType.Text = zoneNode.CurrentZone.Type.ToString();
            this.ExpiresAfter.Text = zoneNode.CurrentZone.ZoneInfo.expire.ToString();
            this.MinimumTTL.Text = zoneNode.CurrentZone.ZoneInfo.minimum.ToString();
        }

        private void label8_Click(object sender, EventArgs e)
        {

        }

        private void EditButton_Click(object sender, EventArgs e)
        {
            if (this.EditButton.Text == VMDNSConstants.EDIT)
            {
                this.EditButton.Text = VMDNSConstants.UPDATE;
                ToggleUIControlsState(true);
            }
            else
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        DoValidateControls();
                        SetZoneValuesFromUIFields();
                        //close dialog and return
                    });
            }
        }

        private void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(PrimaryServerName.Text) || string.IsNullOrWhiteSpace(AdminEmail.Text) ||
                string.IsNullOrWhiteSpace(SerialNumber.Text) || string.IsNullOrWhiteSpace(RefreshInterval.Text) ||
                string.IsNullOrWhiteSpace(RetryInterval.Text) || string.IsNullOrWhiteSpace(ZoneType.Text) ||
                string.IsNullOrWhiteSpace(ExpiresAfter.Text) || string.IsNullOrWhiteSpace(MinimumTTL.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY);
        }

        private void ToggleUIControlsState(bool state)
        {
            PrimaryServerName.Enabled = state;
            AdminEmail.Enabled = state;
            SerialNumber.Enabled = state;
            RefreshInterval.Enabled = state;
            RetryInterval.Enabled = state;
            ExpiresAfter.Enabled = state;
            MinimumTTL.Enabled = state;
        }


        private void SetZoneValuesFromUIFields()
        {
            zoneNode.CurrentZone.DNSName = PrimaryServerName.Text;
            zoneNode.CurrentZone.AdminEmail = AdminEmail.Text;
            zoneNode.CurrentZone.Serial = Convert.ToUInt32(SerialNumber.Text);
            zoneNode.CurrentZone.RefreshInterval = Convert.ToUInt32(RefreshInterval.Text);
            zoneNode.CurrentZone.RetryInterval = Convert.ToUInt32(RetryInterval.Text);
            zoneNode.CurrentZone.ExpiresAfter = Convert.ToUInt32(ExpiresAfter.Text);
            zoneNode.CurrentZone.MinimumTTL = Convert.ToUInt32(MinimumTTL.Text);
        }

        private void CloseButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }
    }
}
