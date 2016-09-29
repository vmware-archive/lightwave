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
using VMDNS.Client;
using System.Net;
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;
using VMDNS.Common;

namespace VMDNS
{
    public partial class ARecordController : RecordControllerBase
    {
        #region Constructors

        // Called when created from unmanaged code
        public ARecordController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public ARecordController(NSCoder coder)
            : base(coder)
        {
        }

        // Call to load from the XIB/NIB file
        public ARecordController()
            : base("ARecord", NSBundle.MainBundle)
        {
        }

        #endregion

        protected override void DoValidateControls()
        {
            if (!VMDNSUtilityService.IsValidIPAddress(HostIPField.StringValue))
                throw new Exception(VMDNSConstants.IP_VALIDATE);

            if (string.IsNullOrWhiteSpace(HostNameField.StringValue))
                throw new ArgumentNullException(VMIdentityConstants.EMPTY_FIELD); 

        }

        public  override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
                {
                    DoValidateControls();
                    var data = new VMDNS_A_DATA();
                    data.IpAddress = (UInt32)IPAddress.NetworkToHostOrder(
                        (int)IPAddress.Parse(HostIPField.StringValue).Address);
                    VMDNS_RECORD_A record = new VMDNS_RECORD_A();
                    record.data = data;
                    record.common.iClass = 1;
                    record.common.pszName = HostNameField.StringValue;
                    record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_A;
                    addressRecord = new VmDnsRecordA(record);

                });
            return addressRecord;
        }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    HostIPField.StringValue = (record as VmDnsRecordA).Address;
                    HostNameField.StringValue = record.Name;
                    TTLLabel.Hidden = false;
                    TTLField.Hidden = false;
                    TTLField.StringValue = record.TTL + "ms";
                });
        }

        public override void SetUIFieldsEditability(bool state)
        {
            HostIPField.Enabled = state;
            HostNameField.Enabled = state;
        }


        //strongly typed view accessor
        public new ARecord View
        {
            get
            {
                return (ARecord)base.View;
            }
        }
    }
}
