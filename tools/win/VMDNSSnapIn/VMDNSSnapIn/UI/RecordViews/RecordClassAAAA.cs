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
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDNS.Client;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.UI.RecordViews
{
    class RecordClassAAAA : RecordClassBase
    {
        public override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                var data = new VMDNS_AAAA_DATA();
                var addr = IPAddress.Parse(AddRecordFrm.AAAARecordHostIP.Text);
                data.Ip6Address.bytes = addr.GetAddressBytes();
                var record = new VMDNS_RECORD_AAAA();
                record.common.iClass = 1;
                record.common.pszName = AddRecordFrm.AAAARecordHostNameText.Text;
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_AAAA;
                record.data = data;
                addressRecord = new VmDnsRecordAAAA(record);

            });
            return addressRecord;
        }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                AddRecordFrm.AAAARecordHostNameText.Text = record.Name;
                AddRecordFrm.AAAARecordHostIP.Text = (record as VmDnsRecordAAAA).Address;
                AddRecordFrm.AAAARecordTTLText.Text = record.TTL + "ms";
            });
        }

        public override void SetUIFieldsEditability(bool state)
        {
            AddRecordFrm.AAAARecordHostIP.Enabled = state;
            AddRecordFrm.AAAARecordHostNameText.Enabled = state;
            AddRecordFrm.AAAARecordTTLLabel.Enabled = state;
            AddRecordFrm.AAAARecordTTLText.Enabled = state;
            AddRecordFrm.AAAARecordTTLLabel.Visible = !state;
            AddRecordFrm.AAAARecordTTLText.Visible = !state;
        }

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(AddRecordFrm.AAAARecordHostIP.Text) 
                || string.IsNullOrWhiteSpace(AddRecordFrm.AAAARecordHostNameText.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY);
        }
    }
}
