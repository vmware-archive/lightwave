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
    class RecordClassNS : RecordClassBase
    {
        public override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                var data = new VMDNS_PTR_DATA();
                data.hostName = AddRecordFrm.NSRecordHostNameText.Text;
                //FQDN check
                if (!data.hostName.EndsWith("."))
                    data.hostName += ".";
                var record = new VMDNS_RECORD_NS();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = AddRecordFrm.NSRecordDomainText.Text;
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_NS;
                addressRecord = new VmDnsRecordNS(record);

            });
            return addressRecord;
        }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                AddRecordFrm.NSRecordDomainText.Text = record.Name;
                AddRecordFrm.NSRecordHostNameText.Text = (record as VmDnsRecordNS).Target;
                AddRecordFrm.NSRecordTTLText.Text = record.TTL + "ms";
            });
        }

        public override void SetUIFieldsEditability(bool state)
        {
            AddRecordFrm.NSRecordDomainText.Enabled = state;
            AddRecordFrm.NSRecordHostNameText.Enabled = state;
            AddRecordFrm.NSRecordTTLLabel.Enabled = state;
            AddRecordFrm.NSRecordTTLText.Enabled = state;
            AddRecordFrm.NSRecordTTLLabel.Visible = !state;
            AddRecordFrm.NSRecordTTLText.Visible = !state;
        }

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(AddRecordFrm.NSRecordHostNameText.Text) || string.IsNullOrWhiteSpace(AddRecordFrm.NSRecordDomainText.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY);
        }
    }
}
