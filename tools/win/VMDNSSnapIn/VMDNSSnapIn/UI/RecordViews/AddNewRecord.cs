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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDNS.Client;
using VMDNSSnapIn.UI.RecordViews;

namespace VMDNSSnapIn.UI
{
    public partial class AddNewRecord : Form
    {
        public System.Windows.Forms.Button CloseButton;
        public System.Windows.Forms.Panel AAAARecord;
        public System.Windows.Forms.TextBox AAAARecordTTLText;
        public System.Windows.Forms.TextBox AAAARecordHostIP;
        public System.Windows.Forms.TextBox AAAARecordHostNameText;

        public System.Windows.Forms.Panel ARecord;
        public System.Windows.Forms.TextBox ARecordTTLText;
        public System.Windows.Forms.TextBox ARecordHostIPText;
        public System.Windows.Forms.TextBox ARecordHostNameText;

        public System.Windows.Forms.Panel CNameRecord;
        public System.Windows.Forms.TextBox CNameRecordHostNameText;
        public System.Windows.Forms.TextBox CNameRecordNameText;

        public System.Windows.Forms.Panel NSRecord;
        public System.Windows.Forms.TextBox NSRecordTTLText;
        public System.Windows.Forms.Label NSRecordTTLLabel;
        public System.Windows.Forms.TextBox NSRecordHostNameText;
        public System.Windows.Forms.TextBox NSRecordDomainText;

        public System.Windows.Forms.Panel PTRRecord;
        public System.Windows.Forms.TextBox PTRRecordHostNameText;
        public System.Windows.Forms.TextBox PTRRecordIPAddressText;
        public System.Windows.Forms.Label label12;
        public System.Windows.Forms.Label label13;
        public System.Windows.Forms.Panel SOARecord;
        public System.Windows.Forms.Panel SRVRecord;
        public System.Windows.Forms.TextBox SRVRecordTargetHostText;
        public System.Windows.Forms.Label label16;
        public System.Windows.Forms.TextBox SRVRecordProtocolNameText;
        public System.Windows.Forms.Label SRVRecordProtocolLabel;
        public System.Windows.Forms.Label SRVRecordServiceLabel;
        public System.Windows.Forms.TextBox SOARecordAdministratorText;
        public System.Windows.Forms.Label label15;
        public System.Windows.Forms.TextBox SOARecordPrimaryServerText;
        public System.Windows.Forms.TextBox SOARecordNameText;
        public System.Windows.Forms.TextBox SRVPortText;
        public System.Windows.Forms.TextBox SRVRecordWeightText;
        public System.Windows.Forms.TextBox SRVRecordPriorityText;
        public System.Windows.Forms.Label AAAARecordTTLLabel;
        public System.Windows.Forms.Label ARecordTTLLabel;
        public System.Windows.Forms.ComboBox SRVRecordProtocolCombo;
        public System.Windows.Forms.Label Name;
        public System.Windows.Forms.ComboBox SRVRecordServiceCombo;

        public VmDnsRecord Record { get; set; }

        RecordType recordType;
        bool isViewMode = false;
        RecordClassBase recordObject;

        Dictionary<RecordType, Panel> RecordPanelDictionary;

        public AddNewRecord(RecordType recordType)
        {
            this.recordType = recordType;
            Record = null;
            InitializeComponent();
            InitialiseUIControls();
        }

        public AddNewRecord(VmDnsRecord record)
        {
            Record = record;
            isViewMode = true;
            recordType = (RecordType)record.Type;
            isViewMode = true;
            InitializeComponent();
            InitialiseUIControls();
        }

       //getters and setters for UI Controls.

        private void InitialiseUIControls()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                RecordPanelDictionary = new Dictionary<RecordType, Panel>() {
            {RecordType.VMDNS_RR_TYPE_A, ARecord},
            {RecordType.VMDNS_RR_TYPE_AAAA, AAAARecord},
            {RecordType.VMDNS_RR_TYPE_CNAME, CNameRecord},
            {RecordType.VMDNS_RR_TYPE_NS, NSRecord},
            {RecordType.VMDNS_RR_TYPE_PTR, PTRRecord},
            {RecordType.VMDNS_RR_TYPE_SRV, SRVRecord},
            {RecordType.VMDNS_RR_TYPE_SOA, SOARecord}
            };
                Panel panel = RecordPanelDictionary[recordType];
                panel.Visible = true;
                
                RecordFactory factory = new RecordFactory();
                recordObject= factory.GetRecord(recordType);
                recordObject.RecordPanel = panel;
                recordObject.AddRecordFrm = this;
                if (isViewMode && recordObject != null)
                {
                    recordObject.SetUIFieldsFromRecordData(Record);
                    AddButton.Visible = false;
                    recordObject.SetUIFieldsEditability(false);
                }
            });
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void label18_Click(object sender, EventArgs e)
        {

        }

        private void CloseButton_Click(object sender, EventArgs e)
        {
            this.Close();
            this.DialogResult = DialogResult.Cancel;
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            this.Record = recordObject.GetRecordDataFromUIFields();
            if (this.Record != null)
            {
                this.Close();
                this.DialogResult = DialogResult.OK;
            }

        }
        
    }
}
