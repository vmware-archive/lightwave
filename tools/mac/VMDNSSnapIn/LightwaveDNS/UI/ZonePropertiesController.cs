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
using Foundation;
using AppKit;
using VmIdentity.UI.Common.Utilities;
using VMDNS.Common;
using VmIdentity.UI.Common;

namespace VMDNS
{
    public partial class ZonePropertiesController : NSWindowController
    {
        private VMDNSZoneEntryNode zoneNode;

        public ZonePropertiesController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public ZonePropertiesController(NSCoder coder)
            : base(coder)
        {
        }

        public ZonePropertiesController(VMDNSZoneEntryNode zoneNode)
            : base("ZoneProperties")
        {
            this.zoneNode = zoneNode;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            UpdateButton.StringValue = VMDNSConstants.EDIT;
            InitialiseUIFieldsFromZoneValues();
        }

        partial void OnClose(Foundation.NSObject sender)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        partial void OnEditUpdate(Foundation.NSObject sender)
        {
            if (UpdateButton.Title == VMDNSConstants.EDIT)
            {
                UpdateButton.Title = VMDNSConstants.UPDATE;
                ToggleUIControlsState(true);
            }
            else
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        DoValidateControls();
                        SetZoneValuesFromUIFields();
                        this.Close();
                        NSApplication.SharedApplication.StopModalWithCode(1);
                    });
            }
        }

        void ToggleUIControlsState(bool state)
        {
            PrimaryServerName.Enabled = state;
            AdminEmail.Enabled = state;
            SerialNumber.Enabled = state;
            RefreshInterval.Enabled = state;
            RetryInterval.Enabled = state;
            ExpiresAfter.Enabled = state;
            MinimumTTL.Enabled = state;
        }

        void InitialiseUIFieldsFromZoneValues()
        {
            this.PrimaryServerName.StringValue = zoneNode.CurrentZone.DNSName;
            this.AdminEmail.StringValue = zoneNode.CurrentZone.AdminEmail;
            this.SerialNumber.StringValue = zoneNode.CurrentZone.Serial.ToString();
            this.RefreshInterval.StringValue = zoneNode.CurrentZone.RefreshInterval.ToString();
            this.RetryInterval.StringValue = zoneNode.CurrentZone.RetryInterval.ToString();
            this.ZoneType.StringValue = zoneNode.CurrentZone.Type.ToString();
            this.ExpiresAfter.StringValue = zoneNode.CurrentZone.ZoneInfo.expire.ToString();
            this.MinimumTTL.StringValue = zoneNode.CurrentZone.ZoneInfo.minimum.ToString();
        }

        void SetZoneValuesFromUIFields()
        {
            zoneNode.CurrentZone.DNSName = PrimaryServerName.StringValue;
            zoneNode.CurrentZone.AdminEmail = AdminEmail.StringValue;
            zoneNode.CurrentZone.Serial = Convert.ToUInt32(SerialNumber.StringValue);
            zoneNode.CurrentZone.RefreshInterval = Convert.ToUInt32(RefreshInterval.StringValue);
            zoneNode.CurrentZone.RetryInterval = Convert.ToUInt32(RetryInterval.StringValue);
            zoneNode.CurrentZone.ExpiresAfter = Convert.ToUInt32(ExpiresAfter.StringValue);
            zoneNode.CurrentZone.MinimumTTL = Convert.ToUInt32(MinimumTTL.StringValue);
        }

        void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(PrimaryServerName.StringValue) || string.IsNullOrWhiteSpace(AdminEmail.StringValue) ||
                string.IsNullOrWhiteSpace(SerialNumber.StringValue) || string.IsNullOrWhiteSpace(RefreshInterval.StringValue) ||
                string.IsNullOrWhiteSpace(RetryInterval.StringValue) || string.IsNullOrWhiteSpace(ZoneType.StringValue) ||
                string.IsNullOrWhiteSpace(ExpiresAfter.StringValue) || string.IsNullOrWhiteSpace(MinimumTTL.StringValue))
                throw new ArgumentNullException(VMIdentityConstants.EMPTY_FIELD);
        }

        public new ZoneProperties Window
        {
            get { return (ZoneProperties)base.Window; }
        }
    }
}
