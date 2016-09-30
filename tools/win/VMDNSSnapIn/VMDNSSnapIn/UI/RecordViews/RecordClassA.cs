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
using VMDNS.Common;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.UI.RecordViews
{
    class RecordClassA : RecordClassBase
    {
        public override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                var data = new VMDNS_A_DATA();
                data.IpAddress = (UInt32)IPAddress.NetworkToHostOrder(
                    (int)IPAddress.Parse(AddRecordFrm.ARecordHostIPText.Text).Address);
                VMDNS_RECORD_A record = new VMDNS_RECORD_A();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = AddRecordFrm.ARecordHostNameText.Text;
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_A;
                addressRecord = new VmDnsRecordA(record);

            });
            return addressRecord;
         }

        public override void SetUIFieldsFromRecordData(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                AddRecordFrm.ARecordHostIPText.Text = (record as VmDnsRecordA).Address;
                AddRecordFrm.ARecordHostNameText.Text = record.Name;
                AddRecordFrm.ARecordTTLText.Text = record.TTL + "ms";
            });
        }

        public override void SetUIFieldsEditability(bool state)
        {
            AddRecordFrm.ARecordHostNameText.Enabled = state;
            AddRecordFrm.ARecordHostIPText.Enabled = state;
            AddRecordFrm.ARecordTTLLabel.Enabled = state;
            AddRecordFrm.ARecordTTLText.Enabled = state;
            AddRecordFrm.ARecordTTLLabel.Visible = !state;
            AddRecordFrm.ARecordTTLText.Visible = !state;
        }

        protected override void DoValidateControls()
        {
            if (!VMDNSUtilityService.IsValidIPAddress(AddRecordFrm.ARecordHostIPText.Text))
                throw new Exception(VMDNSConstants.IP_VALIDATE);

            if (string.IsNullOrWhiteSpace(AddRecordFrm.ARecordHostNameText.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY);
        }
    }
}
