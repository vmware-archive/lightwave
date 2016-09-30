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
using VmIdentity.UI.Common;
using VMDNS.Client;
using VmIdentity.UI.Common.Utilities;


/*
 * Controller class for the SOA Record View
 *
 * @author Sumalatha Abhishek
 */

namespace VMDNS
{
    public partial class SOARecordController : RecordControllerBase
    {
        #region Constructors

        // Called when created from unmanaged code
        public SOARecordController(IntPtr handle)
            : base(handle)
        {
            Initialize();
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public SOARecordController(NSCoder coder)
            : base(coder)
        {
            Initialize();
        }

        // Call to load from the XIB/NIB file
        public SOARecordController()
            : base("SOARecord", NSBundle.MainBundle)
        {
            Initialize();
        }

        // Shared initialization code
        void Initialize()
        {
        }

        #endregion

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(RecordNameField.StringValue) || string.IsNullOrWhiteSpace(ServerField.StringValue) ||
                string.IsNullOrWhiteSpace(AdministratorField.StringValue))
                throw new ArgumentNullException(VMIdentityConstants.EMPTY_FIELD);
        }

        public  override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
                {
                    DoValidateControls();
                    var data = new VMDNS_SOA_DATA();
                    data.pNamePrimaryServer = ServerField.StringValue;
                    data.pNameAdministrator = AdministratorField.StringValue;
                    var record = new VMDNS_RECORD_SOA();
                    record.data = data;
                    record.common.iClass = 1;
                    record.common.pszName = RecordNameField.StringValue;
                    record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_SOA;
                    addressRecord = new VmDnsRecordSOA(record);

                });
            return addressRecord;
        }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            VmDnsRecordSOA srvRecord = record as VmDnsRecordSOA;
            if (srvRecord != null)
            {
                RecordNameField.StringValue = srvRecord.Name;
                AdministratorField.StringValue = srvRecord.RECORD.data.pNameAdministrator;
                ServerField.StringValue = srvRecord.RECORD.data.pNamePrimaryServer;
            }
            else
                UIErrorHelper.ShowAlert("", "Unknown Record Format");
        }

        public override void SetUIFieldsEditability(bool state)
        {
            RecordNameField.Enabled = state;
            AdministratorField.Enabled = state;
            ServerField.Enabled = state;
        }

        //strongly typed view accessor
        public new SOARecord View
        {
            get
            {
                return (SOARecord)base.View;
            }
        }
    }
}
