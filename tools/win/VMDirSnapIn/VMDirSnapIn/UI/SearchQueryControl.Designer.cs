/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System.Windows.Forms;
namespace VMDirSnapIn.UI
{
    partial class SearchQueryControl
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
            this.components = new System.ComponentModel.Container();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.buildFilterPage = new System.Windows.Forms.TabPage();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.comboBoxLogicalOp = new System.Windows.Forms.ComboBox();
            this.listViewConditions = new System.Windows.Forms.ListView();
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.removeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.comboBoxAttr = new System.Windows.Forms.ComboBox();
            this.textBoxVal = new System.Windows.Forms.TextBox();
            this.comboBoxCond = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.buttonAdd = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxBase = new System.Windows.Forms.TextBox();
            this.comboBoxScope = new System.Windows.Forms.ComboBox();
            this.buttonView = new System.Windows.Forms.Button();
            this.buttonSearch = new System.Windows.Forms.Button();
            this.textFilterPage = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.textBoxBase2 = new System.Windows.Forms.TextBox();
            this.comboBoxScope2 = new System.Windows.Forms.ComboBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.textBoxFilterString = new System.Windows.Forms.TextBox();
            this.buttonSearch2 = new System.Windows.Forms.Button();
            this.tabControl1.SuspendLayout();
            this.buildFilterPage.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.contextMenuStrip1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.textFilterPage.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.buildFilterPage);
            this.tabControl1.Controls.Add(this.textFilterPage);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Multiline = true;
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(930, 270);
            this.tabControl1.TabIndex = 9;
            // 
            // buildFilterPage
            // 
            this.buildFilterPage.Controls.Add(this.groupBox3);
            this.buildFilterPage.Controls.Add(this.groupBox2);
            this.buildFilterPage.Controls.Add(this.groupBox1);
            this.buildFilterPage.Controls.Add(this.buttonView);
            this.buildFilterPage.Controls.Add(this.buttonSearch);
            this.buildFilterPage.Location = new System.Drawing.Point(4, 22);
            this.buildFilterPage.Name = "buildFilterPage";
            this.buildFilterPage.Padding = new System.Windows.Forms.Padding(3);
            this.buildFilterPage.Size = new System.Drawing.Size(922, 244);
            this.buildFilterPage.TabIndex = 0;
            this.buildFilterPage.Text = "Build Filter";
            this.buildFilterPage.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.groupBox3.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox3.Controls.Add(this.comboBoxLogicalOp);
            this.groupBox3.Controls.Add(this.listViewConditions);
            this.groupBox3.Location = new System.Drawing.Point(310, 9);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(603, 190);
            this.groupBox3.TabIndex = 20;
            this.groupBox3.TabStop = false;
            // 
            // comboBoxLogicalOp
            // 
            this.comboBoxLogicalOp.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxLogicalOp.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxLogicalOp.FormattingEnabled = true;
            this.comboBoxLogicalOp.Location = new System.Drawing.Point(6, 16);
            this.comboBoxLogicalOp.Name = "comboBoxLogicalOp";
            this.comboBoxLogicalOp.Size = new System.Drawing.Size(73, 21);
            this.comboBoxLogicalOp.TabIndex = 17;
            // 
            // listViewConditions
            // 
            this.listViewConditions.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
            this.listViewConditions.ContextMenuStrip = this.contextMenuStrip1;
            this.listViewConditions.FullRowSelect = true;
            this.listViewConditions.GridLines = true;
            this.listViewConditions.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.listViewConditions.HideSelection = false;
            this.listViewConditions.HoverSelection = true;
            this.listViewConditions.Location = new System.Drawing.Point(6, 43);
            this.listViewConditions.Name = "listViewConditions";
            this.listViewConditions.Size = new System.Drawing.Size(591, 141);
            this.listViewConditions.TabIndex = 9;
            this.listViewConditions.UseCompatibleStateImageBehavior = false;
            this.listViewConditions.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Attribute";
            this.columnHeader2.Width = 170;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Condition";
            this.columnHeader3.Width = 157;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Value";
            this.columnHeader4.Width = 166;
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.removeToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(118, 26);
            this.contextMenuStrip1.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip1_Opening_1);
            // 
            // removeToolStripMenuItem
            // 
            this.removeToolStripMenuItem.Name = "removeToolStripMenuItem";
            this.removeToolStripMenuItem.Size = new System.Drawing.Size(117, 22);
            this.removeToolStripMenuItem.Text = "Remove";
            this.removeToolStripMenuItem.Click += new System.EventHandler(this.removeToolStripMenuItem_Click_1);
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.groupBox2.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox2.Controls.Add(this.comboBoxAttr);
            this.groupBox2.Controls.Add(this.textBoxVal);
            this.groupBox2.Controls.Add(this.comboBoxCond);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.buttonAdd);
            this.groupBox2.Location = new System.Drawing.Point(9, 96);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(286, 141);
            this.groupBox2.TabIndex = 19;
            this.groupBox2.TabStop = false;
            // 
            // comboBoxAttr
            // 
            this.comboBoxAttr.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxAttr.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxAttr.FormattingEnabled = true;
            this.comboBoxAttr.Location = new System.Drawing.Point(91, 19);
            this.comboBoxAttr.Name = "comboBoxAttr";
            this.comboBoxAttr.Size = new System.Drawing.Size(179, 21);
            this.comboBoxAttr.TabIndex = 9;
            // 
            // textBoxVal
            // 
            this.textBoxVal.Location = new System.Drawing.Point(91, 87);
            this.textBoxVal.Name = "textBoxVal";
            this.textBoxVal.Size = new System.Drawing.Size(179, 20);
            this.textBoxVal.TabIndex = 11;
            // 
            // comboBoxCond
            // 
            this.comboBoxCond.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxCond.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxCond.FormattingEnabled = true;
            this.comboBoxCond.Location = new System.Drawing.Point(91, 51);
            this.comboBoxCond.Name = "comboBoxCond";
            this.comboBoxCond.Size = new System.Drawing.Size(179, 21);
            this.comboBoxCond.TabIndex = 10;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(10, 94);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(37, 13);
            this.label6.TabIndex = 11;
            this.label6.Text = "Value:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(10, 59);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(54, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "Condition:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(10, 27);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(49, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "Attribute:";
            // 
            // buttonAdd
            // 
            this.buttonAdd.Location = new System.Drawing.Point(118, 116);
            this.buttonAdd.Name = "buttonAdd";
            this.buttonAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonAdd.TabIndex = 12;
            this.buttonAdd.Text = "Add";
            this.buttonAdd.UseVisualStyleBackColor = true;
            this.buttonAdd.Click += new System.EventHandler(this.buttonAdd_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.groupBox1.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.textBoxBase);
            this.groupBox1.Controls.Add(this.comboBoxScope);
            this.groupBox1.Location = new System.Drawing.Point(9, 9);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(286, 81);
            this.groupBox1.TabIndex = 18;
            this.groupBox1.TabStop = false;
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(70, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Search From:";
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 59);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(78, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Search Scope:";
            // 
            // textBoxBase
            // 
            this.textBoxBase.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.textBoxBase.Location = new System.Drawing.Point(89, 17);
            this.textBoxBase.Name = "textBoxBase";
            this.textBoxBase.Size = new System.Drawing.Size(180, 20);
            this.textBoxBase.TabIndex = 3;
            // 
            // comboBoxScope
            // 
            this.comboBoxScope.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.comboBoxScope.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxScope.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxScope.FormattingEnabled = true;
            this.comboBoxScope.Location = new System.Drawing.Point(90, 51);
            this.comboBoxScope.Name = "comboBoxScope";
            this.comboBoxScope.Size = new System.Drawing.Size(180, 21);
            this.comboBoxScope.TabIndex = 5;
            // 
            // buttonView
            // 
            this.buttonView.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.buttonView.Location = new System.Drawing.Point(626, 212);
            this.buttonView.Name = "buttonView";
            this.buttonView.Size = new System.Drawing.Size(75, 23);
            this.buttonView.TabIndex = 14;
            this.buttonView.Text = "View";
            this.buttonView.UseVisualStyleBackColor = true;
            this.buttonView.Click += new System.EventHandler(this.buttonView_Click_1);
            // 
            // buttonSearch
            // 
            this.buttonSearch.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.buttonSearch.Location = new System.Drawing.Point(538, 212);
            this.buttonSearch.Name = "buttonSearch";
            this.buttonSearch.Size = new System.Drawing.Size(75, 23);
            this.buttonSearch.TabIndex = 15;
            this.buttonSearch.Text = "Search";
            this.buttonSearch.UseVisualStyleBackColor = true;
            this.buttonSearch.Click += new System.EventHandler(this.buttonSearch_Click_1);
            // 
            // textFilterPage
            // 
            this.textFilterPage.Controls.Add(this.groupBox4);
            this.textFilterPage.Controls.Add(this.groupBox5);
            this.textFilterPage.Controls.Add(this.buttonSearch2);
            this.textFilterPage.Location = new System.Drawing.Point(4, 22);
            this.textFilterPage.Name = "textFilterPage";
            this.textFilterPage.Padding = new System.Windows.Forms.Padding(3);
            this.textFilterPage.Size = new System.Drawing.Size(922, 244);
            this.textFilterPage.TabIndex = 1;
            this.textFilterPage.Text = "TextFilter";
            this.textFilterPage.UseVisualStyleBackColor = true;
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.groupBox4.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox4.Controls.Add(this.label1);
            this.groupBox4.Controls.Add(this.label7);
            this.groupBox4.Controls.Add(this.textBoxBase2);
            this.groupBox4.Controls.Add(this.comboBoxScope2);
            this.groupBox4.Location = new System.Drawing.Point(9, 9);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(286, 81);
            this.groupBox4.TabIndex = 26;
            this.groupBox4.TabStop = false;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 24);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Search From:";
            // 
            // label7
            // 
            this.label7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(9, 59);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(78, 13);
            this.label7.TabIndex = 4;
            this.label7.Text = "Search Scope:";
            // 
            // textBoxBase2
            // 
            this.textBoxBase2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.textBoxBase2.Location = new System.Drawing.Point(89, 17);
            this.textBoxBase2.Name = "textBoxBase2";
            this.textBoxBase2.Size = new System.Drawing.Size(180, 20);
            this.textBoxBase2.TabIndex = 3;
            // 
            // comboBoxScope2
            // 
            this.comboBoxScope2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.comboBoxScope2.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxScope2.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxScope2.FormattingEnabled = true;
            this.comboBoxScope2.Location = new System.Drawing.Point(90, 51);
            this.comboBoxScope2.Name = "comboBoxScope2";
            this.comboBoxScope2.Size = new System.Drawing.Size(180, 21);
            this.comboBoxScope2.TabIndex = 5;
            // 
            // groupBox5
            // 
            this.groupBox5.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.groupBox5.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox5.Controls.Add(this.textBoxFilterString);
            this.groupBox5.Location = new System.Drawing.Point(310, 9);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(603, 190);
            this.groupBox5.TabIndex = 25;
            this.groupBox5.TabStop = false;
            // 
            // textBoxFilterString
            // 
            this.textBoxFilterString.Location = new System.Drawing.Point(6, 11);
            this.textBoxFilterString.Multiline = true;
            this.textBoxFilterString.Name = "textBoxFilterString";
            this.textBoxFilterString.Size = new System.Drawing.Size(591, 172);
            this.textBoxFilterString.TabIndex = 18;
            // 
            // buttonSearch2
            // 
            this.buttonSearch2.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.buttonSearch2.Location = new System.Drawing.Point(538, 212);
            this.buttonSearch2.Name = "buttonSearch2";
            this.buttonSearch2.Size = new System.Drawing.Size(75, 23);
            this.buttonSearch2.TabIndex = 19;
            this.buttonSearch2.Text = "Search";
            this.buttonSearch2.UseVisualStyleBackColor = true;
            this.buttonSearch2.Click += new System.EventHandler(this.buttonSearch2_Click);
            // 
            // SearchQueryControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tabControl1);
            this.Name = "SearchQueryControl";
            this.Size = new System.Drawing.Size(930, 270);
            this.tabControl1.ResumeLayout(false);
            this.buildFilterPage.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.contextMenuStrip1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.textFilterPage.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage buildFilterPage;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ComboBox comboBoxLogicalOp;
        private System.Windows.Forms.ListView listViewConditions;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.ComboBox comboBoxAttr;
        private System.Windows.Forms.TextBox textBoxVal;
        private System.Windows.Forms.ComboBox comboBoxCond;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button buttonAdd;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxBase;
        private System.Windows.Forms.ComboBox comboBoxScope;
        private System.Windows.Forms.Button buttonView;
        private System.Windows.Forms.Button buttonSearch;
        private System.Windows.Forms.TabPage textFilterPage;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.TextBox textBoxFilterString;
        private System.Windows.Forms.Button buttonSearch2;
        private GroupBox groupBox4;
        private Label label1;
        private Label label7;
        private TextBox textBoxBase2;
        private ComboBox comboBoxScope2;
        private ContextMenuStrip contextMenuStrip1;
        private ToolStripMenuItem removeToolStripMenuItem;
    }
}
