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
    partial class HttpTransportFormView
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
            this.lblTotalItems = new System.Windows.Forms.Label();
            this.lblItemCount = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.SuspendLayout();
            // 
            // propGridInput
            // 
            this.propGridInput.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.helpProvider1.SetHelpKeyword(this.propGridInput, "HTTPTransportLog");
            this.helpProvider1.SetHelpString(this.propGridInput, "The complete log of the data exchanged between the client and the server over HTT" +
        "P");
            this.propGridInput.Location = new System.Drawing.Point(34, 17);
            this.propGridInput.Name = "propGridInput";
            this.propGridInput.PropertySort = System.Windows.Forms.PropertySort.NoSort;
            this.helpProvider1.SetShowHelp(this.propGridInput, true);
            this.propGridInput.Size = new System.Drawing.Size(909, 359);
            this.propGridInput.TabIndex = 1;
            this.toolTip1.SetToolTip(this.propGridInput, "The complete log of the data exchanged between the client and the server over HTT" +
        "P");
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.helpProvider1.SetHelpString(this.btnCancel, "Click to close the HTTPTransport log go back to the snapin");
            this.btnCancel.Location = new System.Drawing.Point(852, 393);
            this.btnCancel.Name = "btnCancel";
            this.helpProvider1.SetShowHelp(this.btnCancel, true);
            this.btnCancel.Size = new System.Drawing.Size(91, 34);
            this.btnCancel.TabIndex = 3;
            this.btnCancel.Text = "&Close";
            this.toolTip1.SetToolTip(this.btnCancel, "Click to close the HTTPTransport log go back to the snapin");
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // lblTotalItems
            // 
            this.lblTotalItems.AutoSize = true;
            this.lblTotalItems.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTotalItems.Location = new System.Drawing.Point(829, 24);
            this.lblTotalItems.Name = "lblTotalItems";
            this.lblTotalItems.Size = new System.Drawing.Size(74, 13);
            this.lblTotalItems.TabIndex = 5;
            this.lblTotalItems.Text = "Total Items:";
            // 
            // lblItemCount
            // 
            this.lblItemCount.AutoSize = true;
            this.lblItemCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblItemCount.Location = new System.Drawing.Point(903, 24);
            this.lblItemCount.Name = "lblItemCount";
            this.lblItemCount.Size = new System.Drawing.Size(0, 13);
            this.lblItemCount.TabIndex = 6;
            // 
            // button1
            // 
            this.helpProvider1.SetHelpString(this.button1, "Sort the items in the log based on the timestamp");
            this.button1.Location = new System.Drawing.Point(88, 17);
            this.button1.Name = "button1";
            this.helpProvider1.SetShowHelp(this.button1, true);
            this.button1.Size = new System.Drawing.Size(104, 23);
            this.button1.TabIndex = 7;
            this.button1.Text = "Sort By Recency";
            this.toolTip1.SetToolTip(this.button1, "Sort the items in the log based on the timestamp");
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 2000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.ReshowDelay = 100;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.toolTip1.ToolTipTitle = "HTTP Transport Log";
            // 
            // HttpTransportFormView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(984, 453);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.lblItemCount);
            this.Controls.Add(this.lblTotalItems);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.propGridInput);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "HttpTransportFormView";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "HTTP Transport Log";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PropertyGrid propGridInput;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Label lblTotalItems;
        private System.Windows.Forms.Label lblItemCount;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.HelpProvider helpProvider1;
    }
}