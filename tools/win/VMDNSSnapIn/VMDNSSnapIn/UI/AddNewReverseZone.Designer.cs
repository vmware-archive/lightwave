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
    partial class AddNewReverseZone
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
            this.OK = new System.Windows.Forms.Button();
            this.CancelButton = new System.Windows.Forms.Button();
            this.AdminEmailText = new System.Windows.Forms.TextBox();
            this.NetworkIDLengthText = new System.Windows.Forms.TextBox();
            this.NetworkIDText = new System.Windows.Forms.TextBox();
            this.HostNameText = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // OK
            // 
            this.OK.Location = new System.Drawing.Point(443, 252);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(75, 23);
            this.OK.TabIndex = 19;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            this.OK.Click += new System.EventHandler(this.OK_Click);
            // 
            // CancelButton
            // 
            this.CancelButton.Location = new System.Drawing.Point(332, 252);
            this.CancelButton.Name = "CancelButton";
            this.CancelButton.Size = new System.Drawing.Size(75, 23);
            this.CancelButton.TabIndex = 18;
            this.CancelButton.Text = "Cancel";
            this.CancelButton.UseVisualStyleBackColor = true;
            this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
            // 
            // AdminEmailText
            // 
            this.AdminEmailText.Location = new System.Drawing.Point(180, 190);
            this.AdminEmailText.Name = "AdminEmailText";
            this.AdminEmailText.Size = new System.Drawing.Size(338, 22);
            this.AdminEmailText.TabIndex = 17;
            // 
            // NetworkIDLengthText
            // 
            this.NetworkIDLengthText.Location = new System.Drawing.Point(180, 138);
            this.NetworkIDLengthText.Name = "NetworkIDLengthText";
            this.NetworkIDLengthText.Size = new System.Drawing.Size(338, 22);
            this.NetworkIDLengthText.TabIndex = 16;
            // 
            // NetworkIDText
            // 
            this.NetworkIDText.Location = new System.Drawing.Point(180, 95);
            this.NetworkIDText.Name = "NetworkIDText";
            this.NetworkIDText.Size = new System.Drawing.Size(338, 22);
            this.NetworkIDText.TabIndex = 15;
            // 
            // HostNameText
            // 
            this.HostNameText.Location = new System.Drawing.Point(180, 48);
            this.HostNameText.Name = "HostNameText";
            this.HostNameText.Size = new System.Drawing.Size(338, 22);
            this.HostNameText.TabIndex = 14;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(37, 190);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(93, 17);
            this.label4.TabIndex = 13;
            this.label4.Text = "Admin Email :";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(38, 141);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(132, 17);
            this.label3.TabIndex = 12;
            this.label3.Text = "Network ID Length :";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(37, 95);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(80, 17);
            this.label2.TabIndex = 11;
            this.label2.Text = "NetworkID :";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(37, 51);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(86, 17);
            this.label1.TabIndex = 10;
            this.label1.Text = "Host Name :";
            // 
            // AddNewReverseZone
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(555, 322);
            this.Controls.Add(this.OK);
            this.Controls.Add(this.CancelButton);
            this.Controls.Add(this.AdminEmailText);
            this.Controls.Add(this.NetworkIDLengthText);
            this.Controls.Add(this.NetworkIDText);
            this.Controls.Add(this.HostNameText);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "AddNewReverseZone";
            this.Text = "AddNewReverseZone";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button OK;
        private System.Windows.Forms.Button CancelButton;
        private System.Windows.Forms.TextBox AdminEmailText;
        private System.Windows.Forms.TextBox NetworkIDLengthText;
        private System.Windows.Forms.TextBox NetworkIDText;
        private System.Windows.Forms.TextBox HostNameText;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
    }
}