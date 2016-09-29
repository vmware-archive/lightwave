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
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;

namespace VMDNS
{
    public partial  class SRVRecordController : RecordControllerBase
    {
        // Called when created from unmanaged code
        public SRVRecordController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public SRVRecordController(NSCoder coder)
            : base(coder)
        {
        }

        // Call to load from the XIB/NIB file
        public SRVRecordController()
            : base("SRVRecord", NSBundle.MainBundle)
        {
        }

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(TargetHostField.StringValue) || string.IsNullOrWhiteSpace(PriorityField.StringValue) ||
                string.IsNullOrWhiteSpace(WeightField.StringValue) || string.IsNullOrWhiteSpace(PortField.StringValue) ||
                string.IsNullOrWhiteSpace(ServiceOptionsField.StringValue) || string.IsNullOrWhiteSpace(ProtocolOptionsField.StringValue))
                throw new ArgumentNullException(VMIdentityConstants.EMPTY_FIELD);
        }

        public  override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
                {
                    DoValidateControls();
                    var data = new VMDNS_SRV_DATA();
                    data.pNameTarget = TargetHostField.StringValue;
                    data.wPriority = Convert.ToUInt16(PriorityField.StringValue);
                    data.wWeight = Convert.ToUInt16(WeightField.StringValue);
                    data.wPort = Convert.ToUInt16(PortField.StringValue);
                    var record = new VMDNS_RECORD_SRV();
                    record.data = data;
                    record.common.iClass = 1;
                    record.common.pszName = "_" + ServiceOptionsField.TitleOfSelectedItem + "._" + ProtocolOptionsField.TitleOfSelectedItem;
                    record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_SRV;
                    addressRecord = new VmDnsRecordSRV(record);

                });
            return addressRecord;
        }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            VmDnsRecordSRV srvRecord = record as VmDnsRecordSRV;
            if (srvRecord != null)
            {
                TargetHostField.StringValue = srvRecord.Target;
                PriorityField.StringValue = srvRecord.Priority.ToString();
                WeightField.StringValue = srvRecord.Weight.ToString();
                PortField.StringValue = srvRecord.Port.ToString();
                RecordNameField.StringValue = srvRecord.Name;
                ServiceOptionsField.Hidden = true;
                ServiceOptionsLabel.Hidden = true;
                ProtocolOptionsField.Hidden = true;
                ProtocolLabel.Hidden = true;
                RecordNameField.Hidden = false;
                RecordNameLabel.Hidden = false;
            }
            else
                UIErrorHelper.ShowAlert("", "Unknown Record Format");
        }

        public override void SetUIFieldsEditability(bool state)
        {
            TargetHostField.Enabled = state;
            PriorityField.Enabled = state;
            WeightField.Enabled = state;
            PortField.Enabled = state;
        }

        //strongly typed view accessor
        public new SRVRecord View
        {
            get
            {
                return (SRVRecord)base.View;
            }
        }
    }
}
