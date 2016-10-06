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
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.removeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.buttonSearch = new System.Windows.Forms.Button();
            this.contextMenuStrip2 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.buttonAttrRemove = new System.Windows.Forms.Button();
            this.buttonAttrRemoveAll = new System.Windows.Forms.Button();
            this.buttonAttrAdd = new System.Windows.Forms.Button();
            this.buttonFromFile = new System.Windows.Forms.Button();
            this.buttonCopyFilter = new System.Windows.Forms.Button();
            this.buttonCondRemove = new System.Windows.Forms.Button();
            this.buttonCondRemoveAll = new System.Windows.Forms.Button();
            this.buttonCondAdd = new System.Windows.Forms.Button();
            this.panel1 = new System.Windows.Forms.Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.comboBoxAttrToReturn = new System.Windows.Forms.ComboBox();
            this.listViewAttrToReturn = new System.Windows.Forms.ListView();
            this.Attribute = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxBase = new System.Windows.Forms.TextBox();
            this.comboBoxScope = new System.Windows.Forms.ComboBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.buildFilterPage = new System.Windows.Forms.TabPage();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.comboBoxLogicalOp = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.textBoxVal = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.comboBoxAttr = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.comboBoxCond = new System.Windows.Forms.ComboBox();
            this.listViewConditions = new System.Windows.Forms.ListView();
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.textFilterPage = new System.Windows.Forms.TabPage();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.textBoxFilterString = new System.Windows.Forms.TextBox();
            this.contextMenuStrip1.SuspendLayout();
            this.contextMenuStrip2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.buildFilterPage.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.textFilterPage.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.SuspendLayout();
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
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(152, 22);
            this.toolStripMenuItem1.Text = "Remove";
            this.toolStripMenuItem1.Click += new System.EventHandler(this.toolStripMenuItem1_Click);
            // 
            // buttonSearch
            // 
            this.buttonSearch.Location = new System.Drawing.Point(455, 270);
            this.buttonSearch.Name = "buttonSearch";
            this.buttonSearch.Size = new System.Drawing.Size(75, 23);
            this.buttonSearch.TabIndex = 8;
            this.buttonSearch.Text = "Search";
            this.buttonSearch.UseVisualStyleBackColor = true;
            this.buttonSearch.Click += new System.EventHandler(this.buttonSearch_Click);
            // 
            // contextMenuStrip2
            // 
            this.contextMenuStrip2.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1});
            this.contextMenuStrip2.Name = "contextMenuStrip1";
            this.contextMenuStrip2.Size = new System.Drawing.Size(118, 26);
            this.contextMenuStrip2.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip2_Opening);
            // 
            // buttonAttrRemove
            // 
            this.buttonAttrRemove.Location = new System.Drawing.Point(203, 57);
            this.buttonAttrRemove.Name = "buttonAttrRemove";
            this.buttonAttrRemove.Size = new System.Drawing.Size(75, 23);
            this.buttonAttrRemove.TabIndex = 12;
            this.buttonAttrRemove.Text = "Remove";
            this.toolTip1.SetToolTip(this.buttonAttrRemove, "Remove attribute from table");
            this.buttonAttrRemove.UseVisualStyleBackColor = true;
            this.buttonAttrRemove.Click += new System.EventHandler(this.buttonAttrRemove_Click);
            // 
            // buttonAttrRemoveAll
            // 
            this.buttonAttrRemoveAll.Location = new System.Drawing.Point(203, 87);
            this.buttonAttrRemoveAll.Name = "buttonAttrRemoveAll";
            this.buttonAttrRemoveAll.Size = new System.Drawing.Size(75, 23);
            this.buttonAttrRemoveAll.TabIndex = 13;
            this.buttonAttrRemoveAll.Text = "Remove All";
            this.toolTip1.SetToolTip(this.buttonAttrRemoveAll, "Remove all attribute from table");
            this.buttonAttrRemoveAll.UseVisualStyleBackColor = true;
            this.buttonAttrRemoveAll.Click += new System.EventHandler(this.buttonAttrRemoveAll_Click);
            // 
            // buttonAttrAdd
            // 
            this.buttonAttrAdd.Location = new System.Drawing.Point(203, 27);
            this.buttonAttrAdd.Name = "buttonAttrAdd";
            this.buttonAttrAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonAttrAdd.TabIndex = 10;
            this.buttonAttrAdd.Text = "Add";
            this.toolTip1.SetToolTip(this.buttonAttrAdd, "Add attribute");
            this.buttonAttrAdd.UseVisualStyleBackColor = true;
            this.buttonAttrAdd.Click += new System.EventHandler(this.buttonAttrAdd_Click);
            // 
            // buttonFromFile
            // 
            this.buttonFromFile.Location = new System.Drawing.Point(596, 153);
            this.buttonFromFile.Name = "buttonFromFile";
            this.buttonFromFile.Size = new System.Drawing.Size(75, 23);
            this.buttonFromFile.TabIndex = 18;
            this.buttonFromFile.Text = "From File";
            this.toolTip1.SetToolTip(this.buttonFromFile, "Add conditions from file.");
            this.buttonFromFile.UseVisualStyleBackColor = true;
            this.buttonFromFile.Click += new System.EventHandler(this.buttonFromFile_Click);
            // 
            // buttonCopyFilter
            // 
            this.buttonCopyFilter.Location = new System.Drawing.Point(595, 123);
            this.buttonCopyFilter.Name = "buttonCopyFilter";
            this.buttonCopyFilter.Size = new System.Drawing.Size(75, 23);
            this.buttonCopyFilter.TabIndex = 17;
            this.buttonCopyFilter.Text = "Copy To TextFilter";
            this.toolTip1.SetToolTip(this.buttonCopyFilter, "Copy filter to TextFilter");
            this.buttonCopyFilter.UseVisualStyleBackColor = true;
            this.buttonCopyFilter.Click += new System.EventHandler(this.buttonCopyFilter_Click);
            // 
            // buttonCondRemove
            // 
            this.buttonCondRemove.Location = new System.Drawing.Point(595, 65);
            this.buttonCondRemove.Name = "buttonCondRemove";
            this.buttonCondRemove.Size = new System.Drawing.Size(75, 23);
            this.buttonCondRemove.TabIndex = 15;
            this.buttonCondRemove.Text = "Remove";
            this.toolTip1.SetToolTip(this.buttonCondRemove, "Remove selected condition from table");
            this.buttonCondRemove.UseVisualStyleBackColor = true;
            this.buttonCondRemove.Click += new System.EventHandler(this.buttonCondRemove_Click);
            // 
            // buttonCondRemoveAll
            // 
            this.buttonCondRemoveAll.Location = new System.Drawing.Point(595, 94);
            this.buttonCondRemoveAll.Name = "buttonCondRemoveAll";
            this.buttonCondRemoveAll.Size = new System.Drawing.Size(75, 23);
            this.buttonCondRemoveAll.TabIndex = 16;
            this.buttonCondRemoveAll.Text = "Remove All";
            this.toolTip1.SetToolTip(this.buttonCondRemoveAll, "Remove all conditions from table");
            this.buttonCondRemoveAll.UseVisualStyleBackColor = true;
            this.buttonCondRemoveAll.Click += new System.EventHandler(this.buttonCondRemoveAll_Click);
            // 
            // buttonCondAdd
            // 
            this.buttonCondAdd.Location = new System.Drawing.Point(595, 37);
            this.buttonCondAdd.Name = "buttonCondAdd";
            this.buttonCondAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonCondAdd.TabIndex = 6;
            this.buttonCondAdd.Text = "Add";
            this.toolTip1.SetToolTip(this.buttonCondAdd, "Add condition");
            this.buttonCondAdd.UseVisualStyleBackColor = true;
            this.buttonCondAdd.Click += new System.EventHandler(this.buttonCondAdd_Click);
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Controls.Add(this.buttonSearch);
            this.panel1.Controls.Add(this.tabControl1);
            this.panel1.Location = new System.Drawing.Point(3, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(1020, 300);
            this.panel1.TabIndex = 22;
            // 
            // groupBox1
            // 
            this.groupBox1.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox1.Controls.Add(this.groupBox2);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.textBoxBase);
            this.groupBox1.Controls.Add(this.comboBoxScope);
            this.groupBox1.Location = new System.Drawing.Point(3, 2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(315, 265);
            this.groupBox1.TabIndex = 23;
            this.groupBox1.TabStop = false;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.comboBoxAttrToReturn);
            this.groupBox2.Controls.Add(this.buttonAttrRemove);
            this.groupBox2.Controls.Add(this.listViewAttrToReturn);
            this.groupBox2.Controls.Add(this.buttonAttrRemoveAll);
            this.groupBox2.Controls.Add(this.buttonAttrAdd);
            this.groupBox2.Location = new System.Drawing.Point(6, 102);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(303, 156);
            this.groupBox2.TabIndex = 26;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Attributes To Return:";
            // 
            // comboBoxAttrToReturn
            // 
            this.comboBoxAttrToReturn.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxAttrToReturn.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxAttrToReturn.FormattingEnabled = true;
            this.comboBoxAttrToReturn.Location = new System.Drawing.Point(6, 29);
            this.comboBoxAttrToReturn.Name = "comboBoxAttrToReturn";
            this.comboBoxAttrToReturn.Size = new System.Drawing.Size(191, 21);
            this.comboBoxAttrToReturn.TabIndex = 9;
            // 
            // listViewAttrToReturn
            // 
            this.listViewAttrToReturn.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.Attribute});
            this.listViewAttrToReturn.ContextMenuStrip = this.contextMenuStrip2;
            this.listViewAttrToReturn.GridLines = true;
            this.listViewAttrToReturn.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.listViewAttrToReturn.Location = new System.Drawing.Point(6, 58);
            this.listViewAttrToReturn.Name = "listViewAttrToReturn";
            this.listViewAttrToReturn.Size = new System.Drawing.Size(191, 88);
            this.listViewAttrToReturn.TabIndex = 11;
            this.listViewAttrToReturn.UseCompatibleStateImageBehavior = false;
            this.listViewAttrToReturn.View = System.Windows.Forms.View.Details;
            // 
            // Attribute
            // 
            this.Attribute.Width = 185;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(11, 27);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(70, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Search From:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(11, 62);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(78, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Search Scope:";
            // 
            // textBoxBase
            // 
            this.textBoxBase.Location = new System.Drawing.Point(91, 20);
            this.textBoxBase.Name = "textBoxBase";
            this.textBoxBase.Size = new System.Drawing.Size(193, 20);
            this.textBoxBase.TabIndex = 1;
            // 
            // comboBoxScope
            // 
            this.comboBoxScope.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxScope.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxScope.FormattingEnabled = true;
            this.comboBoxScope.Location = new System.Drawing.Point(92, 54);
            this.comboBoxScope.Name = "comboBoxScope";
            this.comboBoxScope.Size = new System.Drawing.Size(192, 21);
            this.comboBoxScope.TabIndex = 2;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.buildFilterPage);
            this.tabControl1.Controls.Add(this.textFilterPage);
            this.tabControl1.Location = new System.Drawing.Point(324, 8);
            this.tabControl1.Multiline = true;
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(693, 259);
            this.tabControl1.TabIndex = 22;
            // 
            // buildFilterPage
            // 
            this.buildFilterPage.Controls.Add(this.groupBox3);
            this.buildFilterPage.Location = new System.Drawing.Point(4, 22);
            this.buildFilterPage.Name = "buildFilterPage";
            this.buildFilterPage.Padding = new System.Windows.Forms.Padding(3);
            this.buildFilterPage.Size = new System.Drawing.Size(685, 233);
            this.buildFilterPage.TabIndex = 0;
            this.buildFilterPage.Text = "Build Filter";
            this.buildFilterPage.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox3.Controls.Add(this.buttonFromFile);
            this.groupBox3.Controls.Add(this.comboBoxLogicalOp);
            this.groupBox3.Controls.Add(this.buttonCopyFilter);
            this.groupBox3.Controls.Add(this.buttonCondRemove);
            this.groupBox3.Controls.Add(this.label6);
            this.groupBox3.Controls.Add(this.buttonCondRemoveAll);
            this.groupBox3.Controls.Add(this.textBoxVal);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Controls.Add(this.comboBoxAttr);
            this.groupBox3.Controls.Add(this.label4);
            this.groupBox3.Controls.Add(this.comboBoxCond);
            this.groupBox3.Controls.Add(this.buttonCondAdd);
            this.groupBox3.Controls.Add(this.listViewConditions);
            this.groupBox3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox3.Location = new System.Drawing.Point(3, 3);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(679, 227);
            this.groupBox3.TabIndex = 20;
            this.groupBox3.TabStop = false;
            // 
            // comboBoxLogicalOp
            // 
            this.comboBoxLogicalOp.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxLogicalOp.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxLogicalOp.FormattingEnabled = true;
            this.comboBoxLogicalOp.Location = new System.Drawing.Point(12, 200);
            this.comboBoxLogicalOp.Name = "comboBoxLogicalOp";
            this.comboBoxLogicalOp.Size = new System.Drawing.Size(58, 21);
            this.comboBoxLogicalOp.TabIndex = 7;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(379, 20);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(37, 13);
            this.label6.TabIndex = 11;
            this.label6.Text = "Value:";
            // 
            // textBoxVal
            // 
            this.textBoxVal.Location = new System.Drawing.Point(382, 37);
            this.textBoxVal.Name = "textBoxVal";
            this.textBoxVal.Size = new System.Drawing.Size(200, 20);
            this.textBoxVal.TabIndex = 5;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(194, 19);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(54, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "Condition:";
            // 
            // comboBoxAttr
            // 
            this.comboBoxAttr.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxAttr.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxAttr.FormattingEnabled = true;
            this.comboBoxAttr.Location = new System.Drawing.Point(12, 37);
            this.comboBoxAttr.Name = "comboBoxAttr";
            this.comboBoxAttr.Size = new System.Drawing.Size(180, 21);
            this.comboBoxAttr.TabIndex = 3;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(9, 19);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(49, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "Attribute:";
            // 
            // comboBoxCond
            // 
            this.comboBoxCond.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxCond.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxCond.FormattingEnabled = true;
            this.comboBoxCond.Location = new System.Drawing.Point(196, 37);
            this.comboBoxCond.Name = "comboBoxCond";
            this.comboBoxCond.Size = new System.Drawing.Size(180, 21);
            this.comboBoxCond.TabIndex = 4;
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
            this.listViewConditions.Location = new System.Drawing.Point(12, 65);
            this.listViewConditions.Name = "listViewConditions";
            this.listViewConditions.Size = new System.Drawing.Size(570, 129);
            this.listViewConditions.TabIndex = 14;
            this.listViewConditions.UseCompatibleStateImageBehavior = false;
            this.listViewConditions.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Attribute";
            this.columnHeader2.Width = 181;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Condition";
            this.columnHeader3.Width = 179;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Value";
            this.columnHeader4.Width = 205;
            // 
            // textFilterPage
            // 
            this.textFilterPage.Controls.Add(this.groupBox5);
            this.textFilterPage.Location = new System.Drawing.Point(4, 22);
            this.textFilterPage.Name = "textFilterPage";
            this.textFilterPage.Padding = new System.Windows.Forms.Padding(3);
            this.textFilterPage.Size = new System.Drawing.Size(685, 233);
            this.textFilterPage.TabIndex = 1;
            this.textFilterPage.Text = "TextFilter";
            this.textFilterPage.UseVisualStyleBackColor = true;
            // 
            // groupBox5
            // 
            this.groupBox5.BackColor = System.Drawing.Color.WhiteSmoke;
            this.groupBox5.Controls.Add(this.textBoxFilterString);
            this.groupBox5.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox5.Location = new System.Drawing.Point(3, 3);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(679, 227);
            this.groupBox5.TabIndex = 25;
            this.groupBox5.TabStop = false;
            // 
            // textBoxFilterString
            // 
            this.textBoxFilterString.Location = new System.Drawing.Point(6, 11);
            this.textBoxFilterString.Multiline = true;
            this.textBoxFilterString.Name = "textBoxFilterString";
            this.textBoxFilterString.Size = new System.Drawing.Size(667, 210);
            this.textBoxFilterString.TabIndex = 18;
            // 
            // SearchQueryControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.panel1);
            this.Name = "SearchQueryControl";
            this.Size = new System.Drawing.Size(1020, 300);
            this.contextMenuStrip1.ResumeLayout(false);
            this.contextMenuStrip2.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.buildFilterPage.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.textFilterPage.ResumeLayout(false);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private ContextMenuStrip contextMenuStrip1;
        private ToolStripMenuItem removeToolStripMenuItem;
        private ToolStripMenuItem toolStripMenuItem1;
        private ToolTip toolTip1;
        private Button buttonSearch;
        private ContextMenuStrip contextMenuStrip2;
        private Panel panel1;
        private GroupBox groupBox1;
        private GroupBox groupBox2;
        private ComboBox comboBoxAttrToReturn;
        private Button buttonAttrRemove;
        private ListView listViewAttrToReturn;
        private ColumnHeader Attribute;
        private Button buttonAttrRemoveAll;
        private Button buttonAttrAdd;
        private Label label2;
        private Label label3;
        private TextBox textBoxBase;
        private ComboBox comboBoxScope;
        private TabControl tabControl1;
        private TabPage buildFilterPage;
        private GroupBox groupBox3;
        private Button buttonFromFile;
        private ComboBox comboBoxLogicalOp;
        private Button buttonCopyFilter;
        private Button buttonCondRemove;
        private Label label6;
        private Button buttonCondRemoveAll;
        private TextBox textBoxVal;
        private Label label5;
        private ComboBox comboBoxAttr;
        private Label label4;
        private ComboBox comboBoxCond;
        private Button buttonCondAdd;
        private ListView listViewConditions;
        private ColumnHeader columnHeader2;
        private ColumnHeader columnHeader3;
        private ColumnHeader columnHeader4;
        private TabPage textFilterPage;
        private GroupBox groupBox5;
        private TextBox textBoxFilterString;
    }
}
