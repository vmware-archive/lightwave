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
    partial class ZoneProperties
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
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.PrimaryServerName = new System.Windows.Forms.TextBox();
            this.AdminEmail = new System.Windows.Forms.TextBox();
            this.SerialNumber = new System.Windows.Forms.TextBox();
            this.ZoneType = new System.Windows.Forms.TextBox();
            this.RefreshInterval = new System.Windows.Forms.TextBox();
            this.RetryInterval = new System.Windows.Forms.TextBox();
            this.ExpiresAfter = new System.Windows.Forms.TextBox();
            this.MinimumTTL = new System.Windows.Forms.TextBox();
            this.EditButton = new System.Windows.Forms.Button();
            this.CloseButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(110, 17);
            this.label1.TabIndex = 0;
            this.label1.Text = "Primary Server :";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 53);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(99, 17);
            this.label2.TabIndex = 1;
            this.label2.Text = "Administrator :";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 94);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(74, 17);
            this.label3.TabIndex = 2;
            this.label3.Text = "Serial No :";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 130);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(48, 17);
            this.label4.TabIndex = 3;
            this.label4.Text = "Type :";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 165);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(120, 17);
            this.label5.TabIndex = 4;
            this.label5.Text = "Refresh Interval : ";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(12, 202);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(104, 17);
            this.label6.TabIndex = 5;
            this.label6.Text = "Retry Interval : ";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(12, 235);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(100, 17);
            this.label7.TabIndex = 6;
            this.label7.Text = "Expires After : ";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(12, 270);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(101, 17);
            this.label8.TabIndex = 7;
            this.label8.Text = "Minimum TTL :";
            this.label8.Click += new System.EventHandler(this.label8_Click);
            // 
            // PrimaryServerName
            // 
            this.PrimaryServerName.Enabled = false;
            this.PrimaryServerName.Location = new System.Drawing.Point(136, 19);
            this.PrimaryServerName.Name = "PrimaryServerName";
            this.PrimaryServerName.Size = new System.Drawing.Size(354, 22);
            this.PrimaryServerName.TabIndex = 8;
            // 
            // AdminEmail
            // 
            this.AdminEmail.Enabled = false;
            this.AdminEmail.Location = new System.Drawing.Point(136, 53);
            this.AdminEmail.Name = "AdminEmail";
            this.AdminEmail.Size = new System.Drawing.Size(354, 22);
            this.AdminEmail.TabIndex = 9;
            // 
            // SerialNumber
            // 
            this.SerialNumber.Enabled = false;
            this.SerialNumber.Location = new System.Drawing.Point(136, 91);
            this.SerialNumber.Name = "SerialNumber";
            this.SerialNumber.Size = new System.Drawing.Size(354, 22);
            this.SerialNumber.TabIndex = 10;
            // 
            // ZoneType
            // 
            this.ZoneType.Enabled = false;
            this.ZoneType.Location = new System.Drawing.Point(136, 130);
            this.ZoneType.Name = "ZoneType";
            this.ZoneType.Size = new System.Drawing.Size(354, 22);
            this.ZoneType.TabIndex = 11;
            // 
            // RefreshInterval
            // 
            this.RefreshInterval.Enabled = false;
            this.RefreshInterval.Location = new System.Drawing.Point(136, 165);
            this.RefreshInterval.Name = "RefreshInterval";
            this.RefreshInterval.Size = new System.Drawing.Size(354, 22);
            this.RefreshInterval.TabIndex = 12;
            // 
            // RetryInterval
            // 
            this.RetryInterval.Enabled = false;
            this.RetryInterval.Location = new System.Drawing.Point(136, 202);
            this.RetryInterval.Name = "RetryInterval";
            this.RetryInterval.Size = new System.Drawing.Size(354, 22);
            this.RetryInterval.TabIndex = 13;
            // 
            // ExpiresAfter
            // 
            this.ExpiresAfter.Enabled = false;
            this.ExpiresAfter.Location = new System.Drawing.Point(136, 230);
            this.ExpiresAfter.Name = "ExpiresAfter";
            this.ExpiresAfter.Size = new System.Drawing.Size(354, 22);
            this.ExpiresAfter.TabIndex = 14;
            // 
            // MinimumTTL
            // 
            this.MinimumTTL.Enabled = false;
            this.MinimumTTL.Location = new System.Drawing.Point(136, 267);
            this.MinimumTTL.Name = "MinimumTTL";
            this.MinimumTTL.Size = new System.Drawing.Size(354, 22);
            this.MinimumTTL.TabIndex = 15;
            // 
            // EditButton
            // 
            this.EditButton.Location = new System.Drawing.Point(317, 317);
            this.EditButton.Name = "EditButton";
            this.EditButton.Size = new System.Drawing.Size(75, 23);
            this.EditButton.TabIndex = 16;
            this.EditButton.Text = "Edit";
            this.EditButton.UseVisualStyleBackColor = true;
            this.EditButton.Click += new System.EventHandler(this.EditButton_Click);
            // 
            // CloseButton
            // 
            this.CloseButton.Location = new System.Drawing.Point(415, 317);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(75, 23);
            this.CloseButton.TabIndex = 17;
            this.CloseButton.Text = "Close";
            this.CloseButton.UseVisualStyleBackColor = true;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // ZoneProperties
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(580, 361);
            this.Controls.Add(this.CloseButton);
            this.Controls.Add(this.EditButton);
            this.Controls.Add(this.MinimumTTL);
            this.Controls.Add(this.ExpiresAfter);
            this.Controls.Add(this.RetryInterval);
            this.Controls.Add(this.RefreshInterval);
            this.Controls.Add(this.ZoneType);
            this.Controls.Add(this.SerialNumber);
            this.Controls.Add(this.AdminEmail);
            this.Controls.Add(this.PrimaryServerName);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "ZoneProperties";
            this.Text = "ZoneProperties";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox PrimaryServerName;
        private System.Windows.Forms.TextBox AdminEmail;
        private System.Windows.Forms.TextBox SerialNumber;
        private System.Windows.Forms.TextBox ZoneType;
        private System.Windows.Forms.TextBox RefreshInterval;
        private System.Windows.Forms.TextBox RetryInterval;
        private System.Windows.Forms.TextBox ExpiresAfter;
        private System.Windows.Forms.TextBox MinimumTTL;
        private System.Windows.Forms.Button EditButton;
        private System.Windows.Forms.Button CloseButton;
    }
}