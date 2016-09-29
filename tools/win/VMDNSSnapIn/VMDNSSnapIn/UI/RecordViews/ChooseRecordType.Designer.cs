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

namespace VMDNSSnapIn.UI.RecordViews
{
    partial class ChooseRecordType
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.ZoneNameText = new System.Windows.Forms.TextBox();
            this.RecordTypeCombo = new System.Windows.Forms.ComboBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(29, 32);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(49, 17);
            this.label1.TabIndex = 0;
            this.label1.Text = "Zone :";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(32, 85);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(98, 17);
            this.label2.TabIndex = 1;
            this.label2.Text = "Record Type :";
            // 
            // ZoneNameText
            // 
            this.ZoneNameText.Enabled = false;
            this.ZoneNameText.Location = new System.Drawing.Point(154, 32);
            this.ZoneNameText.Name = "ZoneNameText";
            this.ZoneNameText.Size = new System.Drawing.Size(312, 22);
            this.ZoneNameText.TabIndex = 2;
            // 
            // RecordTypeCombo
            // 
            this.RecordTypeCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.RecordTypeCombo.FormattingEnabled = true;
            this.RecordTypeCombo.Items.AddRange(new object[] {
            "A",
            "AAAA",
            "SRV",
            "NS",
            "PTR",
            "CNAME"});
            this.RecordTypeCombo.Location = new System.Drawing.Point(154, 82);
            this.RecordTypeCombo.Name = "RecordTypeCombo";
            this.RecordTypeCombo.Size = new System.Drawing.Size(312, 24);
            this.RecordTypeCombo.TabIndex = 3;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(391, 136);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 4;
            this.button1.Text = "Close";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.OnCloseButton);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(283, 136);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 5;
            this.button2.Text = "OK";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.OnOKButton);
            // 
            // ChooseRecordType
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(496, 204);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.RecordTypeCombo);
            this.Controls.Add(this.ZoneNameText);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "ChooseRecordType";
            this.Text = "ChooseRecordType";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox ZoneNameText;
        private System.Windows.Forms.ComboBox RecordTypeCombo;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
    }
}