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

namespace VMDNSSnapIn.UI
{
    partial class AddNewRecord
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AddNewRecord));
            this.CloseButton = new System.Windows.Forms.Button();
            this.AAAARecord = new System.Windows.Forms.Panel();
            this.AAAARecordTTLText = new System.Windows.Forms.TextBox();
            this.AAAARecordHostIP = new System.Windows.Forms.TextBox();
            this.AAAARecordHostNameText = new System.Windows.Forms.TextBox();
            this.AAAARecordTTLLabel = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.ARecord = new System.Windows.Forms.Panel();
            this.ARecordTTLText = new System.Windows.Forms.TextBox();
            this.ARecordHostIPText = new System.Windows.Forms.TextBox();
            this.ARecordHostNameText = new System.Windows.Forms.TextBox();
            this.ARecordTTLLabel = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.PTRRecord = new System.Windows.Forms.Panel();
            this.PTRRecordHostNameText = new System.Windows.Forms.TextBox();
            this.PTRRecordIPAddressText = new System.Windows.Forms.TextBox();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.SOARecord = new System.Windows.Forms.Panel();
            this.SOARecordAdministratorText = new System.Windows.Forms.TextBox();
            this.label15 = new System.Windows.Forms.Label();
            this.SOARecordPrimaryServerText = new System.Windows.Forms.TextBox();
            this.SOARecordNameText = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.SRVRecord = new System.Windows.Forms.Panel();
            this.Name = new System.Windows.Forms.Label();
            this.SRVRecordProtocolCombo = new System.Windows.Forms.ComboBox();
            this.SRVRecordServiceCombo = new System.Windows.Forms.ComboBox();
            this.SRVPortText = new System.Windows.Forms.TextBox();
            this.SRVRecordWeightText = new System.Windows.Forms.TextBox();
            this.SRVRecordPriorityText = new System.Windows.Forms.TextBox();
            this.label21 = new System.Windows.Forms.Label();
            this.label20 = new System.Windows.Forms.Label();
            this.label19 = new System.Windows.Forms.Label();
            this.SRVRecordTargetHostText = new System.Windows.Forms.TextBox();
            this.label16 = new System.Windows.Forms.Label();
            this.SRVRecordProtocolNameText = new System.Windows.Forms.TextBox();
            this.SRVRecordProtocolLabel = new System.Windows.Forms.Label();
            this.SRVRecordServiceLabel = new System.Windows.Forms.Label();
            this.CNameRecord = new System.Windows.Forms.Panel();
            this.CNameRecordHostNameText = new System.Windows.Forms.TextBox();
            this.CNameRecordNameText = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.NSRecord = new System.Windows.Forms.Panel();
            this.NSRecordTTLText = new System.Windows.Forms.TextBox();
            this.NSRecordTTLLabel = new System.Windows.Forms.Label();
            this.NSRecordHostNameText = new System.Windows.Forms.TextBox();
            this.NSRecordDomainText = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.AddButton = new System.Windows.Forms.Button();
            this.AAAARecord.SuspendLayout();
            this.ARecord.SuspendLayout();
            this.PTRRecord.SuspendLayout();
            this.SOARecord.SuspendLayout();
            this.SRVRecord.SuspendLayout();
            this.CNameRecord.SuspendLayout();
            this.NSRecord.SuspendLayout();
            this.SuspendLayout();
            // 
            // CloseButton
            // 
            resources.ApplyResources(this.CloseButton, "CloseButton");
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.UseVisualStyleBackColor = true;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // AAAARecord
            // 
            resources.ApplyResources(this.AAAARecord, "AAAARecord");
            this.AAAARecord.Controls.Add(this.AAAARecordTTLText);
            this.AAAARecord.Controls.Add(this.AAAARecordHostIP);
            this.AAAARecord.Controls.Add(this.AAAARecordHostNameText);
            this.AAAARecord.Controls.Add(this.AAAARecordTTLLabel);
            this.AAAARecord.Controls.Add(this.label2);
            this.AAAARecord.Controls.Add(this.label1);
            this.AAAARecord.Name = "AAAARecord";
            // 
            // AAAARecordTTLText
            // 
            resources.ApplyResources(this.AAAARecordTTLText, "AAAARecordTTLText");
            this.AAAARecordTTLText.Name = "AAAARecordTTLText";
            // 
            // AAAARecordHostIP
            // 
            resources.ApplyResources(this.AAAARecordHostIP, "AAAARecordHostIP");
            this.AAAARecordHostIP.Name = "AAAARecordHostIP";
            // 
            // AAAARecordHostNameText
            // 
            resources.ApplyResources(this.AAAARecordHostNameText, "AAAARecordHostNameText");
            this.AAAARecordHostNameText.Name = "AAAARecordHostNameText";
            // 
            // AAAARecordTTLLabel
            // 
            resources.ApplyResources(this.AAAARecordTTLLabel, "AAAARecordTTLLabel");
            this.AAAARecordTTLLabel.Name = "AAAARecordTTLLabel";
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.Name = "label2";
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.Name = "label1";
            // 
            // ARecord
            // 
            resources.ApplyResources(this.ARecord, "ARecord");
            this.ARecord.Controls.Add(this.ARecordTTLText);
            this.ARecord.Controls.Add(this.ARecordHostIPText);
            this.ARecord.Controls.Add(this.ARecordHostNameText);
            this.ARecord.Controls.Add(this.ARecordTTLLabel);
            this.ARecord.Controls.Add(this.label5);
            this.ARecord.Controls.Add(this.label6);
            this.ARecord.Name = "ARecord";
            // 
            // ARecordTTLText
            // 
            resources.ApplyResources(this.ARecordTTLText, "ARecordTTLText");
            this.ARecordTTLText.Name = "ARecordTTLText";
            // 
            // ARecordHostIPText
            // 
            resources.ApplyResources(this.ARecordHostIPText, "ARecordHostIPText");
            this.ARecordHostIPText.Name = "ARecordHostIPText";
            // 
            // ARecordHostNameText
            // 
            resources.ApplyResources(this.ARecordHostNameText, "ARecordHostNameText");
            this.ARecordHostNameText.Name = "ARecordHostNameText";
            // 
            // ARecordTTLLabel
            // 
            resources.ApplyResources(this.ARecordTTLLabel, "ARecordTTLLabel");
            this.ARecordTTLLabel.Name = "ARecordTTLLabel";
            // 
            // label5
            // 
            resources.ApplyResources(this.label5, "label5");
            this.label5.Name = "label5";
            // 
            // label6
            // 
            resources.ApplyResources(this.label6, "label6");
            this.label6.Name = "label6";
            // 
            // PTRRecord
            // 
            resources.ApplyResources(this.PTRRecord, "PTRRecord");
            this.PTRRecord.Controls.Add(this.PTRRecordHostNameText);
            this.PTRRecord.Controls.Add(this.PTRRecordIPAddressText);
            this.PTRRecord.Controls.Add(this.label12);
            this.PTRRecord.Controls.Add(this.label13);
            this.PTRRecord.Name = "PTRRecord";
            // 
            // PTRRecordHostNameText
            // 
            resources.ApplyResources(this.PTRRecordHostNameText, "PTRRecordHostNameText");
            this.PTRRecordHostNameText.Name = "PTRRecordHostNameText";
            // 
            // PTRRecordIPAddressText
            // 
            resources.ApplyResources(this.PTRRecordIPAddressText, "PTRRecordIPAddressText");
            this.PTRRecordIPAddressText.Name = "PTRRecordIPAddressText";
            // 
            // label12
            // 
            resources.ApplyResources(this.label12, "label12");
            this.label12.Name = "label12";
            // 
            // label13
            // 
            resources.ApplyResources(this.label13, "label13");
            this.label13.AccessibleRole = System.Windows.Forms.AccessibleRole.TitleBar;
            this.label13.Name = "label13";
            // 
            // SOARecord
            // 
            resources.ApplyResources(this.SOARecord, "SOARecord");
            this.SOARecord.Controls.Add(this.SOARecordAdministratorText);
            this.SOARecord.Controls.Add(this.label15);
            this.SOARecord.Controls.Add(this.SOARecordPrimaryServerText);
            this.SOARecord.Controls.Add(this.SOARecordNameText);
            this.SOARecord.Controls.Add(this.label11);
            this.SOARecord.Controls.Add(this.label14);
            this.SOARecord.Name = "SOARecord";
            // 
            // SOARecordAdministratorText
            // 
            resources.ApplyResources(this.SOARecordAdministratorText, "SOARecordAdministratorText");
            this.SOARecordAdministratorText.Name = "SOARecordAdministratorText";
            // 
            // label15
            // 
            resources.ApplyResources(this.label15, "label15");
            this.label15.Name = "label15";
            // 
            // SOARecordPrimaryServerText
            // 
            resources.ApplyResources(this.SOARecordPrimaryServerText, "SOARecordPrimaryServerText");
            this.SOARecordPrimaryServerText.Name = "SOARecordPrimaryServerText";
            // 
            // SOARecordNameText
            // 
            resources.ApplyResources(this.SOARecordNameText, "SOARecordNameText");
            this.SOARecordNameText.Name = "SOARecordNameText";
            // 
            // label11
            // 
            resources.ApplyResources(this.label11, "label11");
            this.label11.Name = "label11";
            // 
            // label14
            // 
            resources.ApplyResources(this.label14, "label14");
            this.label14.AccessibleRole = System.Windows.Forms.AccessibleRole.TitleBar;
            this.label14.Name = "label14";
            // 
            // SRVRecord
            // 
            resources.ApplyResources(this.SRVRecord, "SRVRecord");
            this.SRVRecord.Controls.Add(this.Name);
            this.SRVRecord.Controls.Add(this.SRVRecordProtocolCombo);
            this.SRVRecord.Controls.Add(this.SRVRecordServiceCombo);
            this.SRVRecord.Controls.Add(this.SRVPortText);
            this.SRVRecord.Controls.Add(this.SRVRecordWeightText);
            this.SRVRecord.Controls.Add(this.SRVRecordPriorityText);
            this.SRVRecord.Controls.Add(this.label21);
            this.SRVRecord.Controls.Add(this.label20);
            this.SRVRecord.Controls.Add(this.label19);
            this.SRVRecord.Controls.Add(this.SRVRecordTargetHostText);
            this.SRVRecord.Controls.Add(this.label16);
            this.SRVRecord.Controls.Add(this.SRVRecordProtocolNameText);
            this.SRVRecord.Controls.Add(this.SRVRecordProtocolLabel);
            this.SRVRecord.Controls.Add(this.SRVRecordServiceLabel);
            this.SRVRecord.Name = "SRVRecord";
            // 
            // Name
            // 
            resources.ApplyResources(this.Name, "Name");
            this.Name.Name = "Name";
            // 
            // SRVRecordProtocolCombo
            // 
            resources.ApplyResources(this.SRVRecordProtocolCombo, "SRVRecordProtocolCombo");
            this.SRVRecordProtocolCombo.FormattingEnabled = true;
            this.SRVRecordProtocolCombo.Items.AddRange(new object[] {
            resources.GetString("SRVRecordProtocolCombo.Items"),
            resources.GetString("SRVRecordProtocolCombo.Items1")});
            this.SRVRecordProtocolCombo.Name = "SRVRecordProtocolCombo";
            // 
            // SRVRecordServiceCombo
            // 
            resources.ApplyResources(this.SRVRecordServiceCombo, "SRVRecordServiceCombo");
            this.SRVRecordServiceCombo.FormattingEnabled = true;
            this.SRVRecordServiceCombo.Items.AddRange(new object[] {
            resources.GetString("SRVRecordServiceCombo.Items"),
            resources.GetString("SRVRecordServiceCombo.Items1")});
            this.SRVRecordServiceCombo.Name = "SRVRecordServiceCombo";
            // 
            // SRVPortText
            // 
            resources.ApplyResources(this.SRVPortText, "SRVPortText");
            this.SRVPortText.Name = "SRVPortText";
            // 
            // SRVRecordWeightText
            // 
            resources.ApplyResources(this.SRVRecordWeightText, "SRVRecordWeightText");
            this.SRVRecordWeightText.Name = "SRVRecordWeightText";
            // 
            // SRVRecordPriorityText
            // 
            resources.ApplyResources(this.SRVRecordPriorityText, "SRVRecordPriorityText");
            this.SRVRecordPriorityText.Name = "SRVRecordPriorityText";
            // 
            // label21
            // 
            resources.ApplyResources(this.label21, "label21");
            this.label21.Name = "label21";
            // 
            // label20
            // 
            resources.ApplyResources(this.label20, "label20");
            this.label20.Name = "label20";
            // 
            // label19
            // 
            resources.ApplyResources(this.label19, "label19");
            this.label19.Name = "label19";
            // 
            // SRVRecordTargetHostText
            // 
            resources.ApplyResources(this.SRVRecordTargetHostText, "SRVRecordTargetHostText");
            this.SRVRecordTargetHostText.Name = "SRVRecordTargetHostText";
            // 
            // label16
            // 
            resources.ApplyResources(this.label16, "label16");
            this.label16.Name = "label16";
            // 
            // SRVRecordProtocolNameText
            // 
            resources.ApplyResources(this.SRVRecordProtocolNameText, "SRVRecordProtocolNameText");
            this.SRVRecordProtocolNameText.Name = "SRVRecordProtocolNameText";
            // 
            // SRVRecordProtocolLabel
            // 
            resources.ApplyResources(this.SRVRecordProtocolLabel, "SRVRecordProtocolLabel");
            this.SRVRecordProtocolLabel.Name = "SRVRecordProtocolLabel";
            // 
            // SRVRecordServiceLabel
            // 
            resources.ApplyResources(this.SRVRecordServiceLabel, "SRVRecordServiceLabel");
            this.SRVRecordServiceLabel.AccessibleRole = System.Windows.Forms.AccessibleRole.TitleBar;
            this.SRVRecordServiceLabel.Name = "SRVRecordServiceLabel";
            this.SRVRecordServiceLabel.Click += new System.EventHandler(this.label18_Click);
            // 
            // CNameRecord
            // 
            resources.ApplyResources(this.CNameRecord, "CNameRecord");
            this.CNameRecord.Controls.Add(this.CNameRecordHostNameText);
            this.CNameRecord.Controls.Add(this.CNameRecordNameText);
            this.CNameRecord.Controls.Add(this.label8);
            this.CNameRecord.Controls.Add(this.label9);
            this.CNameRecord.Name = "CNameRecord";
            this.CNameRecord.Paint += new System.Windows.Forms.PaintEventHandler(this.panel1_Paint);
            // 
            // CNameRecordHostNameText
            // 
            resources.ApplyResources(this.CNameRecordHostNameText, "CNameRecordHostNameText");
            this.CNameRecordHostNameText.Name = "CNameRecordHostNameText";
            // 
            // CNameRecordNameText
            // 
            resources.ApplyResources(this.CNameRecordNameText, "CNameRecordNameText");
            this.CNameRecordNameText.Name = "CNameRecordNameText";
            // 
            // label8
            // 
            resources.ApplyResources(this.label8, "label8");
            this.label8.Name = "label8";
            // 
            // label9
            // 
            resources.ApplyResources(this.label9, "label9");
            this.label9.Name = "label9";
            // 
            // NSRecord
            // 
            resources.ApplyResources(this.NSRecord, "NSRecord");
            this.NSRecord.Controls.Add(this.NSRecordTTLText);
            this.NSRecord.Controls.Add(this.NSRecordTTLLabel);
            this.NSRecord.Controls.Add(this.NSRecordHostNameText);
            this.NSRecord.Controls.Add(this.NSRecordDomainText);
            this.NSRecord.Controls.Add(this.label7);
            this.NSRecord.Controls.Add(this.label10);
            this.NSRecord.Name = "NSRecord";
            // 
            // NSRecordTTLText
            // 
            resources.ApplyResources(this.NSRecordTTLText, "NSRecordTTLText");
            this.NSRecordTTLText.Name = "NSRecordTTLText";
            // 
            // NSRecordTTLLabel
            // 
            resources.ApplyResources(this.NSRecordTTLLabel, "NSRecordTTLLabel");
            this.NSRecordTTLLabel.Name = "NSRecordTTLLabel";
            // 
            // NSRecordHostNameText
            // 
            resources.ApplyResources(this.NSRecordHostNameText, "NSRecordHostNameText");
            this.NSRecordHostNameText.Name = "NSRecordHostNameText";
            // 
            // NSRecordDomainText
            // 
            resources.ApplyResources(this.NSRecordDomainText, "NSRecordDomainText");
            this.NSRecordDomainText.Name = "NSRecordDomainText";
            // 
            // label7
            // 
            resources.ApplyResources(this.label7, "label7");
            this.label7.Name = "label7";
            // 
            // label10
            // 
            resources.ApplyResources(this.label10, "label10");
            this.label10.AccessibleRole = System.Windows.Forms.AccessibleRole.TitleBar;
            this.label10.Name = "label10";
            // 
            // AddButton
            // 
            resources.ApplyResources(this.AddButton, "AddButton");
            this.AddButton.Name = "AddButton";
            this.AddButton.UseVisualStyleBackColor = true;
            this.AddButton.Click += new System.EventHandler(this.AddButton_Click);
            // 
            // AddNewRecord
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.AddButton);
            this.Controls.Add(this.SRVRecord);
            this.Controls.Add(this.PTRRecord);
            this.Controls.Add(this.NSRecord);
            this.Controls.Add(this.SOARecord);
            this.Controls.Add(this.CNameRecord);
            this.Controls.Add(this.ARecord);
            this.Controls.Add(this.AAAARecord);
            this.Controls.Add(this.CloseButton);
            //this.Name = "AddNewRecord";
            this.AAAARecord.ResumeLayout(false);
            this.AAAARecord.PerformLayout();
            this.ARecord.ResumeLayout(false);
            this.ARecord.PerformLayout();
            this.PTRRecord.ResumeLayout(false);
            this.PTRRecord.PerformLayout();
            this.SOARecord.ResumeLayout(false);
            this.SOARecord.PerformLayout();
            this.SRVRecord.ResumeLayout(false);
            this.SRVRecord.PerformLayout();
            this.CNameRecord.ResumeLayout(false);
            this.CNameRecord.PerformLayout();
            this.NSRecord.ResumeLayout(false);
            this.NSRecord.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.Button AddButton;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label14;

    }
}