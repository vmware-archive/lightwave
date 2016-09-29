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
    partial class AdvancedExternalDomain
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
            this.btnCreate = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.lblSettings = new System.Windows.Forms.Label();
            this.cbDNForNestedGroups = new System.Windows.Forms.CheckBox();
            this.cbGroupSearch = new System.Windows.Forms.CheckBox();
            this.cbMatchRuleInChain = new System.Windows.Forms.CheckBox();
            this.label15 = new System.Windows.Forms.Label();
            this.dgAttributes = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnRemove = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.label16 = new System.Windows.Forms.Label();
            this.dgAttributeMapContext = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.dgAttributeMap = new System.Windows.Forms.DataGridView();
            this.attribute = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Mapping = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.dgAttributeMapContext.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributeMap)).BeginInit();
            this.SuspendLayout();
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnCreate.Location = new System.Drawing.Point(214, 341);
            this.btnCreate.Name = "btnCreate";
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 39;
            this.btnCreate.Text = "&Add";
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreate_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(295, 341);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 40;
            this.btnClose.Text = "Cl&ose";
            this.btnClose.UseVisualStyleBackColor = true;
            this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutomaticDelay = 200;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.toolTip1.ToolTipTitle = "External Domain";
            // 
            // lblSettings
            // 
            this.lblSettings.AutoSize = true;
            this.lblSettings.Location = new System.Drawing.Point(1, 10);
            this.lblSettings.Name = "lblSettings";
            this.lblSettings.Size = new System.Drawing.Size(48, 13);
            this.lblSettings.TabIndex = 50;
            this.lblSettings.Text = "Settings:";
            // 
            // cbDNForNestedGroups
            // 
            this.cbDNForNestedGroups.AutoSize = true;
            this.cbDNForNestedGroups.Location = new System.Drawing.Point(207, 12);
            this.cbDNForNestedGroups.Name = "cbDNForNestedGroups";
            this.cbDNForNestedGroups.Size = new System.Drawing.Size(158, 17);
            this.cbDNForNestedGroups.TabIndex = 49;
            this.cbDNForNestedGroups.Text = "Base DN for Nested Groups";
            this.cbDNForNestedGroups.UseVisualStyleBackColor = true;
            // 
            // cbGroupSearch
            // 
            this.cbGroupSearch.AutoSize = true;
            this.cbGroupSearch.Location = new System.Drawing.Point(67, 12);
            this.cbGroupSearch.Name = "cbGroupSearch";
            this.cbGroupSearch.Size = new System.Drawing.Size(92, 17);
            this.cbGroupSearch.TabIndex = 48;
            this.cbGroupSearch.Text = "Group Search";
            this.cbGroupSearch.UseVisualStyleBackColor = true;
            // 
            // cbMatchRuleInChain
            // 
            this.cbMatchRuleInChain.AutoSize = true;
            this.cbMatchRuleInChain.Location = new System.Drawing.Point(68, 35);
            this.cbMatchRuleInChain.Name = "cbMatchRuleInChain";
            this.cbMatchRuleInChain.Size = new System.Drawing.Size(135, 17);
            this.cbMatchRuleInChain.TabIndex = 47;
            this.cbMatchRuleInChain.Text = "Matching Rule in chain";
            this.cbMatchRuleInChain.UseVisualStyleBackColor = true;
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(6, 210);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(121, 13);
            this.label15.TabIndex = 39;
            this.label15.Text = "Shema Object Mapping:";
            // 
            // dgAttributes
            // 
            this.dgAttributes.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.dgAttributes.FullRowSelect = true;
            this.dgAttributes.GridLines = true;
            this.dgAttributes.Location = new System.Drawing.Point(5, 226);
            this.dgAttributes.MultiSelect = false;
            this.dgAttributes.Name = "dgAttributes";
            this.dgAttributes.Size = new System.Drawing.Size(335, 98);
            this.dgAttributes.TabIndex = 36;
            this.dgAttributes.UseCompatibleStateImageBehavior = false;
            this.dgAttributes.View = System.Windows.Forms.View.Details;
            this.dgAttributes.SelectedIndexChanged += new System.EventHandler(this.dgAttributes_SelectedIndexChanged);
            this.dgAttributes.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.dgAttributes_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Object ID - Class Name";
            this.columnHeader1.Width = 300;
            // 
            // btnRemove
            // 
            this.btnRemove.Location = new System.Drawing.Point(346, 299);
            this.btnRemove.Name = "btnRemove";
            this.btnRemove.Size = new System.Drawing.Size(25, 25);
            this.btnRemove.TabIndex = 38;
            this.btnRemove.Text = "-";
            this.btnRemove.UseVisualStyleBackColor = true;
            this.btnRemove.Click += new System.EventHandler(this.btnRemove_Click);
            // 
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(346, 226);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(25, 25);
            this.btnAdd.TabIndex = 37;
            this.btnAdd.Text = "+";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(2, 55);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(73, 13);
            this.label16.TabIndex = 46;
            this.label16.Text = "Attribute Map:";
            // 
            // dgAttributeMapContext
            // 
            this.dgAttributeMapContext.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1});
            this.dgAttributeMapContext.Name = "dgContext";
            this.dgAttributeMapContext.Size = new System.Drawing.Size(108, 26);
            this.dgAttributeMapContext.Click += new System.EventHandler(this.dgAttributeMapContext_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(107, 22);
            this.toolStripMenuItem1.Text = "Delete";
            // 
            // dgAttributeMap
            // 
            this.dgAttributeMap.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgAttributeMap.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.attribute,
            this.Mapping});
            this.dgAttributeMap.ContextMenuStrip = this.dgAttributeMapContext;
            this.dgAttributeMap.Location = new System.Drawing.Point(5, 71);
            this.dgAttributeMap.MultiSelect = false;
            this.dgAttributeMap.Name = "dgAttributeMap";
            this.dgAttributeMap.Size = new System.Drawing.Size(366, 126);
            this.dgAttributeMap.TabIndex = 35;
            // 
            // attribute
            // 
            this.attribute.HeaderText = "Attribute";
            this.attribute.Name = "attribute";
            this.attribute.Width = 200;
            // 
            // Mapping
            // 
            this.Mapping.HeaderText = "Mapping";
            this.Mapping.Name = "Mapping";
            this.Mapping.Width = 120;
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Location = new System.Drawing.Point(7, -23);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(0, 10);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Location = new System.Drawing.Point(11, 134);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(0, 10);
            this.groupBox1.TabIndex = 17;
            this.groupBox1.TabStop = false;
            // 
            // AdvancedExternalDomain
            // 
            this.AcceptButton = this.btnCreate;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(377, 370);
            this.Controls.Add(this.lblSettings);
            this.Controls.Add(this.cbDNForNestedGroups);
            this.Controls.Add(this.cbGroupSearch);
            this.Controls.Add(this.dgAttributeMap);
            this.Controls.Add(this.cbMatchRuleInChain);
            this.Controls.Add(this.label16);
            this.Controls.Add(this.dgAttributes);
            this.Controls.Add(this.btnRemove);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.label15);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AdvancedExternalDomain";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Advanced Settings";
            this.Load += new System.EventHandler(this.NewExternalDomain_Load);
            this.dgAttributeMapContext.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributeMap)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.ListView dgAttributes;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Button btnRemove;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.ContextMenuStrip dgAttributeMapContext;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.DataGridView dgAttributeMap;
        private System.Windows.Forms.DataGridViewTextBoxColumn attribute;
        private System.Windows.Forms.DataGridViewTextBoxColumn Mapping;
        private System.Windows.Forms.CheckBox cbMatchRuleInChain;
        private System.Windows.Forms.CheckBox cbGroupSearch;
        private System.Windows.Forms.CheckBox cbDNForNestedGroups;
        private System.Windows.Forms.Label lblSettings;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox1;

    }
}