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
    class RecordClassSRV : RecordClassBase
    {
        public override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                var data = new VMDNS_SRV_DATA();
                data.pNameTarget = AddRecordFrm.SRVRecordTargetHostText.Text;
                data.wPriority = Convert.ToUInt16(AddRecordFrm.SRVRecordPriorityText.Text);
                data.wWeight = Convert.ToUInt16(AddRecordFrm.SRVRecordWeightText.Text);
                data.wPort = Convert.ToUInt16(AddRecordFrm.SRVPortText.Text);
                var record = new VMDNS_RECORD_SRV();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = "_" + AddRecordFrm.SRVRecordServiceCombo.Text + "._" + AddRecordFrm.SRVRecordProtocolCombo.Text;
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
                AddRecordFrm.SRVRecordTargetHostText.Text = srvRecord.Target;
                AddRecordFrm.SRVRecordPriorityText.Text = srvRecord.Priority.ToString();
                AddRecordFrm.SRVRecordWeightText.Text = srvRecord.Weight.ToString();
                AddRecordFrm.SRVPortText.Text = srvRecord.Port.ToString();
                AddRecordFrm.SRVRecordProtocolNameText.Visible = true;
                AddRecordFrm.SRVRecordProtocolNameText.Text = srvRecord.Name;
                AddRecordFrm.SRVRecordProtocolLabel.Visible = true;
                AddRecordFrm.SRVRecordProtocolCombo.Visible = false;
                AddRecordFrm.SRVRecordServiceCombo.Visible = false;
                AddRecordFrm.SRVRecordServiceLabel.Visible = false;

            }
           else
               UIErrorHelper.ShowMessage("Unknown Record Format");
        }

        public override void SetUIFieldsEditability(bool state)
        {
            AddRecordFrm.SRVRecordTargetHostText.Enabled = state;
            AddRecordFrm.SRVRecordPriorityText.Enabled = state;
            AddRecordFrm.SRVRecordWeightText.Enabled = state;
            AddRecordFrm.SRVPortText.Enabled = state;
        }

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(AddRecordFrm.SRVRecordTargetHostText.Text)
                || string.IsNullOrWhiteSpace(AddRecordFrm.SRVRecordPriorityText.Text) 
                || string.IsNullOrWhiteSpace(AddRecordFrm.SRVRecordWeightText.Text) || string.IsNullOrWhiteSpace(AddRecordFrm.SRVPortText.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY);
        }
    }
}
