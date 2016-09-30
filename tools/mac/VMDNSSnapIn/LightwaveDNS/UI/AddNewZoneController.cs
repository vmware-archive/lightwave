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
using VMDNS.Client;
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;
using VMDNS.Common;

namespace VMDNS
{
    public partial class AddNewZoneController : NSWindowController
    {
        private int zoneType;

        public  VMDNS_ZONE_INFO ZoneInfo;

        public AddNewZoneController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public AddNewZoneController(NSCoder coder)
            : base(coder)
        {
        }

        public AddNewZoneController(int type)
            : base("AddNewZone")
        {
            zoneType = type;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            if (zoneType == (int)VmDnsZoneType.FORWARD)
                NewZoneContainerBox.ContentView = ForwardZoneBox.ContentView;
            else
                NewZoneContainerBox.ContentView = ReverseZoneBox.ContentView;

        }

        void DoValidateControls()
        {
            switch (zoneType)
            {
                case (int)VmDnsZoneType.FORWARD:
                    if (!VMDNSUtilityService.IsValidIPAddress(HostIPForwardZone.StringValue))
                        throw new Exception(VMDNSConstants.IP_VALIDATE);

                    if (string.IsNullOrWhiteSpace(HostNameForwardZone.StringValue) ||
                        string.IsNullOrWhiteSpace(ZoneNameForwardZone.StringValue))
                        throw new Exception("One or more required fields are empty");
                    break;
                case (int)VmDnsZoneType.REVERSE:
                    if (string.IsNullOrWhiteSpace(HostNameReverseZone.StringValue) || string.IsNullOrWhiteSpace(NetworkIDReverseZone.StringValue) ||
                        string.IsNullOrWhiteSpace(NoBitsReverseZone.StringValue))
                        throw new Exception("One or more required fields are empty");
                    break;
                default:
                    break;
            }
        }

        partial void OnAddZone(Foundation.NSObject sender)
        {
            UIErrorHelper.CheckedExec(delegate()
                {

                    DoValidateControls();

                    switch (zoneType)
                    {
                        case (int)VmDnsZoneType.FORWARD:
                            ZoneInfo.pszName = ZoneNameForwardZone.StringValue;
                            ZoneInfo.pszRName = AdminEmailForwardZone.StringValue;
                            ZoneInfo.pszPrimaryDnsSrvName = HostNameForwardZone.StringValue;
                            ZoneInfo.dwZoneType = (UInt32)VmDnsZoneType.FORWARD;
                            break;
                        case (int)VmDnsZoneType.REVERSE:
                            ZoneInfo.pszRName = AdminEmailReverseZone.StringValue;
                            ZoneInfo.pszName = HostNameReverseZone.StringValue;
                            ZoneInfo.pszPrimaryDnsSrvName = NetworkIDReverseZone.StringValue + "\\" + NoBitsReverseZone.StringValue;
                            ZoneInfo.dwZoneType = (UInt32)VmDnsZoneType.REVERSE;
                            break;
                    }
                    this.Close();
                    NSApplication.SharedApplication.StopModalWithCode(VMIdentityConstants.DIALOGOK);
                });
           

        }

        partial void OnCancel(Foundation.NSObject sender)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(VMIdentityConstants.DIALOGCANCEL);

        }

        public new AddNewZone Window
        {
            get { return (AddNewZone)base.Window; }
        }
    }
}
