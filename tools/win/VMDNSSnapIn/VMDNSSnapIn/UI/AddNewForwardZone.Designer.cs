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
    partial class AddNewForwardZone
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
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.HostNameText = new System.Windows.Forms.TextBox();
            this.HostIPText = new System.Windows.Forms.TextBox();
            this.AdminEmailText = new System.Windows.Forms.TextBox();
            this.ZoneNameText = new System.Windows.Forms.TextBox();
            this.CancelButton = new System.Windows.Forms.Button();
            this.OK = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(52, 42);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(86, 17);
            this.label1.TabIndex = 0;
            this.label1.Text = "Host Name :";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(51, 86);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(57, 17);
            this.label2.TabIndex = 1;
            this.label2.Text = "Host IP:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(52, 132);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(89, 17);
            this.label3.TabIndex = 2;
            this.label3.Text = "Admin Email:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(51, 181);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(90, 17);
            this.label4.TabIndex = 3;
            this.label4.Text = "Zone Name :";
            // 
            // HostNameText
            // 
            this.HostNameText.Location = new System.Drawing.Point(194, 39);
            this.HostNameText.Name = "HostNameText";
            this.HostNameText.Size = new System.Drawing.Size(338, 22);
            this.HostNameText.TabIndex = 4;
            // 
            // HostIPText
            // 
            this.HostIPText.Location = new System.Drawing.Point(194, 86);
            this.HostIPText.Name = "HostIPText";
            this.HostIPText.Size = new System.Drawing.Size(338, 22);
            this.HostIPText.TabIndex = 5;
            // 
            // AdminEmailText
            // 
            this.AdminEmailText.Location = new System.Drawing.Point(194, 129);
            this.AdminEmailText.Name = "AdminEmailText";
            this.AdminEmailText.Size = new System.Drawing.Size(338, 22);
            this.AdminEmailText.TabIndex = 6;
            // 
            // ZoneNameText
            // 
            this.ZoneNameText.Location = new System.Drawing.Point(194, 181);
            this.ZoneNameText.Name = "ZoneNameText";
            this.ZoneNameText.Size = new System.Drawing.Size(338, 22);
            this.ZoneNameText.TabIndex = 7;
            // 
            // CancelButton
            // 
            this.CancelButton.Location = new System.Drawing.Point(346, 243);
            this.CancelButton.Name = "CancelButton";
            this.CancelButton.Size = new System.Drawing.Size(75, 23);
            this.CancelButton.TabIndex = 8;
            this.CancelButton.Text = "Cancel";
            this.CancelButton.UseVisualStyleBackColor = true;
            this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
            // 
            // OK
            // 
            this.OK.Location = new System.Drawing.Point(457, 243);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(75, 23);
            this.OK.TabIndex = 9;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            this.OK.Click += new System.EventHandler(this.OK_Click);
            // 
            // AddNewForwardZone
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(561, 315);
            this.Controls.Add(this.OK);
            this.Controls.Add(this.CancelButton);
            this.Controls.Add(this.ZoneNameText);
            this.Controls.Add(this.AdminEmailText);
            this.Controls.Add(this.HostIPText);
            this.Controls.Add(this.HostNameText);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "AddNewForwardZone";
            this.Text = "AddNewForwardZone";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox HostNameText;
        private System.Windows.Forms.TextBox HostIPText;
        private System.Windows.Forms.TextBox AdminEmailText;
        private System.Windows.Forms.TextBox ZoneNameText;
        private System.Windows.Forms.Button CancelButton;
        private System.Windows.Forms.Button OK;
    }
}