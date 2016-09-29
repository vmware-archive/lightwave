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
 
namespace VMPscHighAvailabilitySnapIn.UI
{
    partial class ServerView
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lstdcs = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.txtServername = new System.Windows.Forms.TextBox();
            this.txtDomainname = new System.Windows.Forms.TextBox();
            this.txtDomainFunctionalLevel = new System.Windows.Forms.TextBox();
            this.pnlGlobalView = new System.Windows.Forms.Panel();
            this.lstNoRecordsView = new System.Windows.Forms.ListView();
            this.columnHeader8 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.pnlGlobalView.SuspendLayout();
            this.SuspendLayout();
            // 
            // lstdcs
            // 
            this.lstdcs.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstdcs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader5,
            this.columnHeader2,
            this.columnHeader3});
            this.lstdcs.FullRowSelect = true;
            this.lstdcs.GridLines = true;
            this.lstdcs.Location = new System.Drawing.Point(19, 153);
            this.lstdcs.MultiSelect = false;
            this.lstdcs.Name = "lstdcs";
            this.lstdcs.Size = new System.Drawing.Size(623, 535);
            this.lstdcs.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lstdcs.TabIndex = 1;
            this.lstdcs.UseCompatibleStateImageBehavior = false;
            this.lstdcs.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Site Name";
            this.columnHeader1.Width = 150;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Host name";
            this.columnHeader5.Width = 260;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Status";
            this.columnHeader2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Node Type";
            this.columnHeader3.Width = 100;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(21, 14);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(48, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Server:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(16, 49);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Domain:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(16, 84);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(151, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Domain Functional Level:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(16, 123);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(103, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "Global Topology:";
            // 
            // txtServername
            // 
            this.txtServername.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtServername.Location = new System.Drawing.Point(71, 14);
            this.txtServername.Name = "txtServername";
            this.txtServername.ReadOnly = true;
            this.txtServername.Size = new System.Drawing.Size(527, 13);
            this.txtServername.TabIndex = 6;
            // 
            // txtDomainname
            // 
            this.txtDomainname.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtDomainname.Location = new System.Drawing.Point(79, 53);
            this.txtDomainname.Name = "txtDomainname";
            this.txtDomainname.ReadOnly = true;
            this.txtDomainname.Size = new System.Drawing.Size(527, 13);
            this.txtDomainname.TabIndex = 7;
            // 
            // txtDomainFunctionalLevel
            // 
            this.txtDomainFunctionalLevel.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtDomainFunctionalLevel.Location = new System.Drawing.Point(177, 88);
            this.txtDomainFunctionalLevel.Name = "txtDomainFunctionalLevel";
            this.txtDomainFunctionalLevel.ReadOnly = true;
            this.txtDomainFunctionalLevel.Size = new System.Drawing.Size(156, 13);
            this.txtDomainFunctionalLevel.TabIndex = 8;
            // 
            // pnlGlobalView
            // 
            this.pnlGlobalView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlGlobalView.Controls.Add(this.lstdcs);
            this.pnlGlobalView.Controls.Add(this.label4);
            this.pnlGlobalView.Controls.Add(this.txtServername);
            this.pnlGlobalView.Controls.Add(this.label3);
            this.pnlGlobalView.Controls.Add(this.label1);
            this.pnlGlobalView.Controls.Add(this.label2);
            this.pnlGlobalView.Location = new System.Drawing.Point(4, 3);
            this.pnlGlobalView.Name = "pnlGlobalView";
            this.pnlGlobalView.Size = new System.Drawing.Size(660, 714);
            this.pnlGlobalView.TabIndex = 9;
            this.pnlGlobalView.Visible = false;
            // 
            // lstNoRecordsView
            // 
            this.lstNoRecordsView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstNoRecordsView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader8});
            this.lstNoRecordsView.FullRowSelect = true;
            this.lstNoRecordsView.GridLines = true;
            this.lstNoRecordsView.Location = new System.Drawing.Point(4, 3);
            this.lstNoRecordsView.MultiSelect = false;
            this.lstNoRecordsView.Name = "lstNoRecordsView";
            this.lstNoRecordsView.Size = new System.Drawing.Size(660, 723);
            this.lstNoRecordsView.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lstNoRecordsView.TabIndex = 7;
            this.lstNoRecordsView.UseCompatibleStateImageBehavior = false;
            this.lstNoRecordsView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "Name";
            this.columnHeader8.Width = 100;
            // 
            // ServerView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.Controls.Add(this.lstNoRecordsView);
            this.Controls.Add(this.txtDomainFunctionalLevel);
            this.Controls.Add(this.txtDomainname);
            this.Controls.Add(this.pnlGlobalView);
            this.Name = "ServerView";
            this.Size = new System.Drawing.Size(667, 732);
            this.Load += new System.EventHandler(this.GlobalView_Load);
            this.pnlGlobalView.ResumeLayout(false);
            this.pnlGlobalView.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView lstdcs;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox txtServername;
        private System.Windows.Forms.TextBox txtDomainname;
        private System.Windows.Forms.TextBox txtDomainFunctionalLevel;
        private System.Windows.Forms.Panel pnlGlobalView;
        private System.Windows.Forms.ListView lstNoRecordsView;
        private System.Windows.Forms.ColumnHeader columnHeader8;
    }
}
