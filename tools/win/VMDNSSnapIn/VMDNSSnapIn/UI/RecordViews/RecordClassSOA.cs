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
    class RecordClassSOA : RecordClassBase
    {
        public override VmDnsRecord GetRecordDataFromUIFields()
        {
            VmDnsRecord addressRecord = null;
            UIErrorHelper.CheckedExec(delegate()
            {
                DoValidateControls();
                var data = new VMDNS_SOA_DATA();
                data.pNamePrimaryServer = AddRecordFrm.SOARecordPrimaryServerText.Text;
                data.pNameAdministrator = AddRecordFrm.SOARecordAdministratorText.Text;
                var record = new VMDNS_RECORD_SOA();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = AddRecordFrm.SOARecordNameText.Text;
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
                AddRecordFrm.SOARecordNameText.Text = srvRecord.Name;
                AddRecordFrm.SOARecordAdministratorText.Text = srvRecord.RECORD.data.pNameAdministrator;
                AddRecordFrm.SOARecordPrimaryServerText.Text = srvRecord.RECORD.data.pNamePrimaryServer;
            }
            else
                UIErrorHelper.ShowError("Unknown Record Format");
        }

        public override void SetUIFieldsEditability(bool state)
        {
            AddRecordFrm.SOARecordNameText.Enabled = state;
            AddRecordFrm.SOARecordAdministratorText.Enabled = state;
            AddRecordFrm.SOARecordPrimaryServerText.Enabled = state;
        }

        protected override void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(AddRecordFrm.SOARecordPrimaryServerText.Text) || string.IsNullOrWhiteSpace(AddRecordFrm.SOARecordAdministratorText.Text) ||
                string.IsNullOrWhiteSpace(AddRecordFrm.SOARecordPrimaryServerText.Text))
                throw new ArgumentNullException(MMCUIConstants.VALUES_EMPTY);
        }
    }
}
