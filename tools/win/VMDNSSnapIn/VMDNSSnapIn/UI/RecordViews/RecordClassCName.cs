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
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDNS.Client;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.UI.RecordViews
{
    class RecordClassCName : RecordClassBase
    {

        public override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                var data = new VMDNS_PTR_DATA();
                data.hostName = AddRecordFrm.CNameRecordHostNameText.Text;
                var record = new VMDNS_RECORD_CNAME();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = AddRecordFrm.CNameRecordNameText.Text;
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_CNAME;
                addressRecord = new VmDnsRecordCNAME(record);

            });
            return addressRecord;
        }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            AddRecordFrm.CNameRecordNameText.Text = record.Name;
            AddRecordFrm.CNameRecordHostNameText.Text = (record is VmDnsRecordCNAME) ? (record as VmDnsRecordCNAME).HostName : null;

        }

        public override void SetUIFieldsEditability(bool state)
        {
            AddRecordFrm.CNameRecordNameText.Enabled = state;
            AddRecordFrm.CNameRecordHostNameText.Enabled = state;
        }

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(AddRecordFrm.CNameRecordNameText.Text) || string.IsNullOrWhiteSpace(AddRecordFrm.CNameRecordHostNameText.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY); 
        }
    }
}
