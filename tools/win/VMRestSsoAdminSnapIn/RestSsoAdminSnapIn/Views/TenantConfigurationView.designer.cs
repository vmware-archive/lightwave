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

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    partial class TenantConfigurationView
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
            this.components = new System.ComponentModel.Container();
            this.propGridInput = new System.Windows.Forms.PropertyGrid();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnApply = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.panel1 = new System.Windows.Forms.Panel();
            this.SuspendLayout();
            // 
            // propGridInput
            // 
            this.helpProvider1.SetHelpKeyword(this.propGridInput, "Tenant Configuration");
            this.helpProvider1.SetHelpString(this.propGridInput, "Configuration of the tenant");
            this.propGridInput.Location = new System.Drawing.Point(14, 12);
            this.propGridInput.Name = "propGridInput";
            this.propGridInput.PropertySort = System.Windows.Forms.PropertySort.NoSort;
            this.helpProvider1.SetShowHelp(this.propGridInput, true);
            this.propGridInput.Size = new System.Drawing.Size(908, 359);
            this.propGridInput.TabIndex = 1;
            this.toolTip1.SetToolTip(this.propGridInput, "The complete log of the data exchanged between the client and the server over HTT" +
        "P");
            this.propGridInput.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this.propGridInput_PropertyValueChanged);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.helpProvider1.SetHelpString(this.btnCancel, "Click to close the HTTPTransport log go back to the snapin");
            this.btnCancel.Location = new System.Drawing.Point(830, 388);
            this.btnCancel.Name = "btnCancel";
            this.helpProvider1.SetShowHelp(this.btnCancel, true);
            this.btnCancel.Size = new System.Drawing.Size(91, 34);
            this.btnCancel.TabIndex = 3;
            this.btnCancel.Text = "&Cancel";
            this.toolTip1.SetToolTip(this.btnCancel, "Click to close the HTTPTransport log go back to the snapin");
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnApply
            // 
            this.helpProvider1.SetHelpKeyword(this.btnApply, "HttpTransportReport");
            this.helpProvider1.SetHelpString(this.btnApply, "Click to view the log in a report format. You can also export the report into por" +
        "table format.");
            this.btnApply.Location = new System.Drawing.Point(733, 388);
            this.btnApply.Name = "btnApply";
            this.helpProvider1.SetShowHelp(this.btnApply, true);
            this.btnApply.Size = new System.Drawing.Size(91, 34);
            this.btnApply.TabIndex = 4;
            this.btnApply.Text = "&Update";
            this.toolTip1.SetToolTip(this.btnApply, "Click to view the log in a report format.");
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Click += new System.EventHandler(this.btnApply_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 2000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.ReshowDelay = 100;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.toolTip1.ToolTipTitle = "HTTP Transport Log";
            // 
            // panel1
            // 
            this.panel1.Location = new System.Drawing.Point(14, 12);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(164, 25);
            this.panel1.TabIndex = 5;
            // 
            // TenantConfigurationView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(937, 434);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnApply);
            this.Controls.Add(this.propGridInput);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "TenantConfigurationView";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Tenant Configuration";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PropertyGrid propGridInput;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnApply;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private System.Windows.Forms.Panel panel1;
    }
}