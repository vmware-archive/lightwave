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
namespace VMDirSnapIn.UI
{
    partial class SuperLogBrowser
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SuperLogBrowser));
            this.lblSuperLogStatus = new System.Windows.Forms.Label();
            this.lvLogInfo = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnSuperLogOnOff = new System.Windows.Forms.Button();
            this.btnClearEntries = new System.Windows.Forms.Button();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnClear = new System.Windows.Forms.Button();
            this.btnFilter = new System.Windows.Forms.Button();
            this.txtFilter = new System.Windows.Forms.TextBox();
            this.cbFilterCriteria = new System.Windows.Forms.ComboBox();
            this.cbFilterColumn = new System.Windows.Forms.ComboBox();
            this.btnChangeBufferSize = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.txtBufferSize = new System.Windows.Forms.NumericUpDown();
            this.txtAutoRefresh = new System.Windows.Forms.NumericUpDown();
            this.chkAutoRefresh = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.timerAutoRefresh = new System.Windows.Forms.Timer(this.components);
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtBufferSize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtAutoRefresh)).BeginInit();
            this.SuspendLayout();
            // 
            // lblSuperLogStatus
            // 
            this.lblSuperLogStatus.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lblSuperLogStatus.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lblSuperLogStatus.Location = new System.Drawing.Point(6, 16);
            this.lblSuperLogStatus.Name = "lblSuperLogStatus";
            this.lblSuperLogStatus.Size = new System.Drawing.Size(620, 25);
            this.lblSuperLogStatus.TabIndex = 0;
            // 
            // lvLogInfo
            // 
            this.lvLogInfo.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lvLogInfo.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader6,
            this.columnHeader7,
            this.columnHeader5,
            this.columnHeader3});
            this.lvLogInfo.FullRowSelect = true;
            this.lvLogInfo.Location = new System.Drawing.Point(13, 143);
            this.lvLogInfo.MultiSelect = false;
            this.lvLogInfo.Name = "lvLogInfo";
            this.lvLogInfo.Size = new System.Drawing.Size(765, 326);
            this.lvLogInfo.TabIndex = 1;
            this.lvLogInfo.UseCompatibleStateImageBehavior = false;
            this.lvLogInfo.View = System.Windows.Forms.View.Details;
            this.lvLogInfo.VirtualMode = true;
            this.lvLogInfo.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lvLogInfo_RetrieveVirtualItem);
            this.lvLogInfo.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lvLogInfo_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Client IP";
            this.columnHeader1.Width = 92;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Port";
            this.columnHeader2.Width = 51;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Login DN";
            this.columnHeader6.Width = 280;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Operation";
            this.columnHeader7.Width = 88;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Error Code";
            this.columnHeader5.Width = 62;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Duration";
            this.columnHeader3.Width = 87;
            // 
            // btnSuperLogOnOff
            // 
            this.btnSuperLogOnOff.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnSuperLogOnOff.Location = new System.Drawing.Point(632, 16);
            this.btnSuperLogOnOff.Name = "btnSuperLogOnOff";
            this.btnSuperLogOnOff.Size = new System.Drawing.Size(127, 23);
            this.btnSuperLogOnOff.TabIndex = 1;
            this.btnSuperLogOnOff.Text = "Turn superlogging on";
            this.btnSuperLogOnOff.UseVisualStyleBackColor = true;
            this.btnSuperLogOnOff.Click += new System.EventHandler(this.btnSuperLogOnOff_Click);
            // 
            // btnClearEntries
            // 
            this.btnClearEntries.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClearEntries.Location = new System.Drawing.Point(632, 53);
            this.btnClearEntries.Name = "btnClearEntries";
            this.btnClearEntries.Size = new System.Drawing.Size(127, 23);
            this.btnClearEntries.TabIndex = 5;
            this.btnClearEntries.Text = "Clear entries";
            this.btnClearEntries.UseVisualStyleBackColor = true;
            this.btnClearEntries.Click += new System.EventHandler(this.btnClearEntries_Click);
            // 
            // btnRefresh
            // 
            this.btnRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRefresh.Location = new System.Drawing.Point(681, 475);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(97, 23);
            this.btnRefresh.TabIndex = 5;
            this.btnRefresh.Text = "Refresh";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.btnClear);
            this.groupBox1.Controls.Add(this.btnFilter);
            this.groupBox1.Controls.Add(this.txtFilter);
            this.groupBox1.Controls.Add(this.cbFilterCriteria);
            this.groupBox1.Controls.Add(this.cbFilterColumn);
            this.groupBox1.Controls.Add(this.btnChangeBufferSize);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.txtBufferSize);
            this.groupBox1.Controls.Add(this.lblSuperLogStatus);
            this.groupBox1.Controls.Add(this.btnSuperLogOnOff);
            this.groupBox1.Controls.Add(this.btnClearEntries);
            this.groupBox1.Location = new System.Drawing.Point(13, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(765, 125);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Superlogging controls";
            // 
            // btnClear
            // 
            this.btnClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClear.Location = new System.Drawing.Point(699, 92);
            this.btnClear.Name = "btnClear";
            this.btnClear.Size = new System.Drawing.Size(60, 23);
            this.btnClear.TabIndex = 10;
            this.btnClear.Text = "Clear";
            this.btnClear.UseVisualStyleBackColor = true;
            this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
            // 
            // btnFilter
            // 
            this.btnFilter.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnFilter.Location = new System.Drawing.Point(632, 92);
            this.btnFilter.Name = "btnFilter";
            this.btnFilter.Size = new System.Drawing.Size(61, 23);
            this.btnFilter.TabIndex = 9;
            this.btnFilter.Text = "Filter";
            this.btnFilter.UseVisualStyleBackColor = true;
            this.btnFilter.Click += new System.EventHandler(this.btnFilter_Click);
            // 
            // txtFilter
            // 
            this.txtFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtFilter.Location = new System.Drawing.Point(357, 95);
            this.txtFilter.Name = "txtFilter";
            this.txtFilter.Size = new System.Drawing.Size(269, 20);
            this.txtFilter.TabIndex = 8;
            // 
            // cbFilterCriteria
            // 
            this.cbFilterCriteria.FormattingEnabled = true;
            this.cbFilterCriteria.Location = new System.Drawing.Point(206, 94);
            this.cbFilterCriteria.Name = "cbFilterCriteria";
            this.cbFilterCriteria.Size = new System.Drawing.Size(145, 21);
            this.cbFilterCriteria.TabIndex = 7;
            // 
            // cbFilterColumn
            // 
            this.cbFilterColumn.FormattingEnabled = true;
            this.cbFilterColumn.Items.AddRange(new object[] {
            "Client IP",
            "Port",
            "Login DN",
            "Operation",
            "Error Code",
            "Duration"});
            this.cbFilterColumn.Location = new System.Drawing.Point(12, 94);
            this.cbFilterColumn.Name = "cbFilterColumn";
            this.cbFilterColumn.Size = new System.Drawing.Size(182, 21);
            this.cbFilterColumn.TabIndex = 6;
            this.cbFilterColumn.SelectedIndexChanged += new System.EventHandler(this.cbFilterColumn_SelectedIndexChanged);
            // 
            // btnChangeBufferSize
            // 
            this.btnChangeBufferSize.Location = new System.Drawing.Point(206, 50);
            this.btnChangeBufferSize.Name = "btnChangeBufferSize";
            this.btnChangeBufferSize.Size = new System.Drawing.Size(145, 23);
            this.btnChangeBufferSize.TabIndex = 4;
            this.btnChangeBufferSize.Text = "Change server buffer size";
            this.btnChangeBufferSize.UseVisualStyleBackColor = true;
            this.btnChangeBufferSize.Click += new System.EventHandler(this.btnChangeBufferSize_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 55);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(92, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Server buffer size:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // txtBufferSize
            // 
            this.txtBufferSize.Location = new System.Drawing.Point(107, 53);
            this.txtBufferSize.Maximum = new decimal(new int[] {
            100000,
            0,
            0,
            0});
            this.txtBufferSize.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.txtBufferSize.Name = "txtBufferSize";
            this.txtBufferSize.Size = new System.Drawing.Size(87, 20);
            this.txtBufferSize.TabIndex = 3;
            this.txtBufferSize.Value = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            // 
            // txtAutoRefresh
            // 
            this.txtAutoRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.txtAutoRefresh.Location = new System.Drawing.Point(134, 481);
            this.txtAutoRefresh.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
            this.txtAutoRefresh.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.txtAutoRefresh.Name = "txtAutoRefresh";
            this.txtAutoRefresh.Size = new System.Drawing.Size(67, 20);
            this.txtAutoRefresh.TabIndex = 3;
            this.txtAutoRefresh.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.txtAutoRefresh.ValueChanged += new System.EventHandler(this.txtAutoRefresh_ValueChanged);
            // 
            // chkAutoRefresh
            // 
            this.chkAutoRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.chkAutoRefresh.AutoSize = true;
            this.chkAutoRefresh.Location = new System.Drawing.Point(13, 481);
            this.chkAutoRefresh.Name = "chkAutoRefresh";
            this.chkAutoRefresh.Size = new System.Drawing.Size(115, 17);
            this.chkAutoRefresh.TabIndex = 2;
            this.chkAutoRefresh.Text = "Auto refresh every ";
            this.chkAutoRefresh.UseVisualStyleBackColor = true;
            this.chkAutoRefresh.CheckedChanged += new System.EventHandler(this.chkAutoRefresh_CheckedChanged);
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(207, 485);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(47, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "seconds";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // timerAutoRefresh
            // 
            this.timerAutoRefresh.Interval = 2000;
            this.timerAutoRefresh.Tick += new System.EventHandler(this.timerAutoRefresh_Tick);
            // 
            // SuperLogBrowser
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(790, 510);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.chkAutoRefresh);
            this.Controls.Add(this.txtAutoRefresh);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.btnRefresh);
            this.Controls.Add(this.lvLogInfo);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "SuperLogBrowser";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Superlogging browser";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtBufferSize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtAutoRefresh)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblSuperLogStatus;
        private System.Windows.Forms.ListView lvLogInfo;
        private System.Windows.Forms.Button btnSuperLogOnOff;
        private System.Windows.Forms.Button btnClearEntries;
        private System.Windows.Forms.Button btnRefresh;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnChangeBufferSize;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown txtBufferSize;
        private System.Windows.Forms.NumericUpDown txtAutoRefresh;
        private System.Windows.Forms.CheckBox chkAutoRefresh;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Timer timerAutoRefresh;
        private System.Windows.Forms.ComboBox cbFilterColumn;
        private System.Windows.Forms.Button btnFilter;
        private System.Windows.Forms.TextBox txtFilter;
        private System.Windows.Forms.ComboBox cbFilterCriteria;
        private System.Windows.Forms.Button btnClear;
    }
}