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
    public partial class CNameRecordController : RecordControllerBase
    {
        #region Constructors

        // Called when created from unmanaged code
        public CNameRecordController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public CNameRecordController(NSCoder coder)
            : base(coder)
        {
        }

        // Call to load from the XIB/NIB file
        public CNameRecordController()
            : base("CNameRecord", NSBundle.MainBundle)
        {
        }

        #endregion

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(NameField.StringValue) || string.IsNullOrWhiteSpace(HostNameField.StringValue))
                throw new ArgumentNullException(VMIdentityConstants.EMPTY_FIELD); 
        }

        public  override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
                {
                    DoValidateControls();
                    var data = new VMDNS_PTR_DATA();
                    data.hostName = HostNameField.StringValue;
                    var record = new VMDNS_RECORD_CNAME();
                    record.data = data;
                    record.common.iClass = 1;
                    record.common.pszName = NameField.StringValue;
                    record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_CNAME;
                    addressRecord = new VmDnsRecordCNAME(record);

                });
            return addressRecord;
        }

        public  override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            NameField.StringValue = record.Name;
            HostNameField.StringValue = (record is VmDnsRecordCNAME) ? (record as VmDnsRecordCNAME).HostName : null;
        }

        public override void SetUIFieldsEditability(bool state)
        {
            NameField.Enabled = state;
            HostNameField.Enabled = state;
        }

        //strongly typed view accessor
        public new CNameRecord View
        {
            get
            {
                return (CNameRecord)base.View;
            }
        }
    }
}
