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
 */namespace VMDirSnapIn.UI
{
    partial class SchemaBrowser
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
            this.lstObjectClasses = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabProperties = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.treeHeirarchy = new System.Windows.Forms.TreeView();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.lstRequiredAttrs = new System.Windows.Forms.ListView();
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.lstOptionalAttrs = new System.Windows.Forms.ListView();
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.tabContentRules = new System.Windows.Forms.TabControl();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.lstContentAux = new System.Windows.Forms.ListView();
            this.columnHeader8 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader9 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader10 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.lstContentMust = new System.Windows.Forms.ListView();
            this.columnHeader11 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader12 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader13 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.lstContentMay = new System.Windows.Forms.ListView();
            this.columnHeader14 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader15 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader16 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabProperties.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.tabContentRules.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.tabPage7.SuspendLayout();
            this.SuspendLayout();
            // 
            // lstObjectClasses
            // 
            this.lstObjectClasses.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstObjectClasses.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lstObjectClasses.FullRowSelect = true;
            this.lstObjectClasses.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.lstObjectClasses.HideSelection = false;
            this.lstObjectClasses.Location = new System.Drawing.Point(13, 13);
            this.lstObjectClasses.MultiSelect = false;
            this.lstObjectClasses.Name = "lstObjectClasses";
            this.lstObjectClasses.Size = new System.Drawing.Size(303, 471);
            this.lstObjectClasses.TabIndex = 0;
            this.lstObjectClasses.UseCompatibleStateImageBehavior = false;
            this.lstObjectClasses.View = System.Windows.Forms.View.Details;
            this.lstObjectClasses.VirtualMode = true;
            this.lstObjectClasses.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstObjectClasses_RetrieveVirtualItem);
            this.lstObjectClasses.SelectedIndexChanged += new System.EventHandler(this.lstObjectClasses_SelectedIndexChanged);
            this.lstObjectClasses.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.lstObjectClasses_OnKeyPress);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Width = 250;
            // 
            // tabProperties
            // 
            this.tabProperties.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabProperties.Controls.Add(this.tabPage1);
            this.tabProperties.Controls.Add(this.tabPage2);
            this.tabProperties.Controls.Add(this.tabPage3);
            this.tabProperties.Controls.Add(this.tabPage4);
            this.tabProperties.Location = new System.Drawing.Point(352, 20);
            this.tabProperties.Name = "tabProperties";
            this.tabProperties.SelectedIndex = 0;
            this.tabProperties.Size = new System.Drawing.Size(429, 471);
            this.tabProperties.TabIndex = 1;
            this.tabProperties.SelectedIndexChanged += new System.EventHandler(this.tabProperties_SelectedIndexChanged);
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.treeHeirarchy);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(421, 445);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Heirarchy";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // treeHeirarchy
            // 
            this.treeHeirarchy.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeHeirarchy.Location = new System.Drawing.Point(3, 3);
            this.treeHeirarchy.Name = "treeHeirarchy";
            this.treeHeirarchy.Size = new System.Drawing.Size(415, 439);
            this.treeHeirarchy.TabIndex = 0;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.lstRequiredAttrs);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(421, 445);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Required Attributes";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // lstRequiredAttrs
            // 
            this.lstRequiredAttrs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
            this.lstRequiredAttrs.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstRequiredAttrs.FullRowSelect = true;
            this.lstRequiredAttrs.Location = new System.Drawing.Point(3, 3);
            this.lstRequiredAttrs.MultiSelect = false;
            this.lstRequiredAttrs.Name = "lstRequiredAttrs";
            this.lstRequiredAttrs.Size = new System.Drawing.Size(415, 439);
            this.lstRequiredAttrs.TabIndex = 0;
            this.lstRequiredAttrs.UseCompatibleStateImageBehavior = false;
            this.lstRequiredAttrs.View = System.Windows.Forms.View.Details;
            this.lstRequiredAttrs.VirtualMode = true;
            this.lstRequiredAttrs.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstRequiredAttrs_RetrieveVirtualItem);
            this.lstRequiredAttrs.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.lstRequiredAttrs_KeyPress);
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Name";
            this.columnHeader2.Width = 114;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Type";
            this.columnHeader3.Width = 110;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Description";
            this.columnHeader4.Width = 139;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.lstOptionalAttrs);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(421, 445);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Optional Attributes";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // lstOptionalAttrs
            // 
            this.lstOptionalAttrs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5,
            this.columnHeader6,
            this.columnHeader7});
            this.lstOptionalAttrs.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstOptionalAttrs.FullRowSelect = true;
            this.lstOptionalAttrs.Location = new System.Drawing.Point(0, 0);
            this.lstOptionalAttrs.MultiSelect = false;
            this.lstOptionalAttrs.Name = "lstOptionalAttrs";
            this.lstOptionalAttrs.Size = new System.Drawing.Size(421, 445);
            this.lstOptionalAttrs.TabIndex = 1;
            this.lstOptionalAttrs.UseCompatibleStateImageBehavior = false;
            this.lstOptionalAttrs.View = System.Windows.Forms.View.Details;
            this.lstOptionalAttrs.VirtualMode = true;
            this.lstOptionalAttrs.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstOptionalAttrs_RetrieveVirtualItem);
            this.lstOptionalAttrs.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.lstOptionalAttrs_KeyPress);
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Name";
            this.columnHeader5.Width = 114;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Type";
            this.columnHeader6.Width = 110;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Description";
            this.columnHeader7.Width = 139;
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.tabContentRules);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(421, 445);
            this.tabPage4.TabIndex = 3;
            this.tabPage4.Text = "Content Rules";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // tabContentRules
            // 
            this.tabContentRules.Alignment = System.Windows.Forms.TabAlignment.Bottom;
            this.tabContentRules.Controls.Add(this.tabPage5);
            this.tabContentRules.Controls.Add(this.tabPage6);
            this.tabContentRules.Controls.Add(this.tabPage7);
            this.tabContentRules.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabContentRules.Location = new System.Drawing.Point(3, 3);
            this.tabContentRules.Name = "tabContentRules";
            this.tabContentRules.SelectedIndex = 0;
            this.tabContentRules.Size = new System.Drawing.Size(415, 439);
            this.tabContentRules.TabIndex = 0;
            this.tabContentRules.SelectedIndexChanged += new System.EventHandler(this.tabContentRules_SelectedIndexChanged);
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.lstContentAux);
            this.tabPage5.Location = new System.Drawing.Point(4, 4);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage5.Size = new System.Drawing.Size(407, 413);
            this.tabPage5.TabIndex = 0;
            this.tabPage5.Text = "Aux";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // lstContentAux
            // 
            this.lstContentAux.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader8,
            this.columnHeader9,
            this.columnHeader10});
            this.lstContentAux.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstContentAux.FullRowSelect = true;
            this.lstContentAux.Location = new System.Drawing.Point(3, 3);
            this.lstContentAux.MultiSelect = false;
            this.lstContentAux.Name = "lstContentAux";
            this.lstContentAux.Size = new System.Drawing.Size(401, 407);
            this.lstContentAux.TabIndex = 2;
            this.lstContentAux.UseCompatibleStateImageBehavior = false;
            this.lstContentAux.View = System.Windows.Forms.View.Details;
            this.lstContentAux.VirtualMode = true;
            this.lstContentAux.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstContentAux_RetrieveVirtualItem);
            this.lstContentAux.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.lstContentAux_KeyPress);
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "Name";
            this.columnHeader8.Width = 114;
            // 
            // columnHeader9
            // 
            this.columnHeader9.Text = "Type";
            this.columnHeader9.Width = 110;
            // 
            // columnHeader10
            // 
            this.columnHeader10.Text = "Description";
            this.columnHeader10.Width = 139;
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.lstContentMust);
            this.tabPage6.Location = new System.Drawing.Point(4, 4);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage6.Size = new System.Drawing.Size(407, 413);
            this.tabPage6.TabIndex = 1;
            this.tabPage6.Text = "Must";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // lstContentMust
            // 
            this.lstContentMust.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader11,
            this.columnHeader12,
            this.columnHeader13});
            this.lstContentMust.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstContentMust.FullRowSelect = true;
            this.lstContentMust.Location = new System.Drawing.Point(3, 3);
            this.lstContentMust.MultiSelect = false;
            this.lstContentMust.Name = "lstContentMust";
            this.lstContentMust.Size = new System.Drawing.Size(401, 407);
            this.lstContentMust.TabIndex = 2;
            this.lstContentMust.UseCompatibleStateImageBehavior = false;
            this.lstContentMust.View = System.Windows.Forms.View.Details;
            this.lstContentMust.VirtualMode = true;
            this.lstContentMust.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstContentMust_RetrieveVirtualItem);
            // 
            // columnHeader11
            // 
            this.columnHeader11.Text = "Name";
            this.columnHeader11.Width = 114;
            // 
            // columnHeader12
            // 
            this.columnHeader12.Text = "Type";
            this.columnHeader12.Width = 110;
            // 
            // columnHeader13
            // 
            this.columnHeader13.Text = "Description";
            this.columnHeader13.Width = 139;
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.lstContentMay);
            this.tabPage7.Location = new System.Drawing.Point(4, 4);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage7.Size = new System.Drawing.Size(407, 413);
            this.tabPage7.TabIndex = 2;
            this.tabPage7.Text = "May";
            this.tabPage7.UseVisualStyleBackColor = true;
            // 
            // lstContentMay
            // 
            this.lstContentMay.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader14,
            this.columnHeader15,
            this.columnHeader16});
            this.lstContentMay.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstContentMay.FullRowSelect = true;
            this.lstContentMay.Location = new System.Drawing.Point(3, 3);
            this.lstContentMay.MultiSelect = false;
            this.lstContentMay.Name = "lstContentMay";
            this.lstContentMay.Size = new System.Drawing.Size(401, 407);
            this.lstContentMay.TabIndex = 2;
            this.lstContentMay.UseCompatibleStateImageBehavior = false;
            this.lstContentMay.View = System.Windows.Forms.View.Details;
            this.lstContentMay.VirtualMode = true;
            this.lstContentMay.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstContentMay_RetrieveVirtualItem);
            // 
            // columnHeader14
            // 
            this.columnHeader14.Text = "Name";
            this.columnHeader14.Width = 114;
            // 
            // columnHeader15
            // 
            this.columnHeader15.Text = "Type";
            this.columnHeader15.Width = 110;
            // 
            // columnHeader16
            // 
            this.columnHeader16.Text = "Description";
            this.columnHeader16.Width = 139;
            // 
            // SchemaBrowser
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(772, 503);
            this.Controls.Add(this.tabProperties);
            this.Controls.Add(this.lstObjectClasses);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SchemaBrowser";
            this.ShowIcon = false;
            this.Text = "SchemaBrowser";
            this.tabProperties.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            this.tabContentRules.ResumeLayout(false);
            this.tabPage5.ResumeLayout(false);
            this.tabPage6.ResumeLayout(false);
            this.tabPage7.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lstObjectClasses;
        private System.Windows.Forms.TabControl tabProperties;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ListView lstRequiredAttrs;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ListView lstOptionalAttrs;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.TreeView treeHeirarchy;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.TabControl tabContentRules;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.TabPage tabPage6;
        private System.Windows.Forms.ListView lstContentAux;
        private System.Windows.Forms.ColumnHeader columnHeader8;
        private System.Windows.Forms.ColumnHeader columnHeader9;
        private System.Windows.Forms.ColumnHeader columnHeader10;
        private System.Windows.Forms.ListView lstContentMust;
        private System.Windows.Forms.ColumnHeader columnHeader11;
        private System.Windows.Forms.ColumnHeader columnHeader12;
        private System.Windows.Forms.ColumnHeader columnHeader13;
        private System.Windows.Forms.TabPage tabPage7;
        private System.Windows.Forms.ListView lstContentMay;
        private System.Windows.Forms.ColumnHeader columnHeader14;
        private System.Windows.Forms.ColumnHeader columnHeader15;
        private System.Windows.Forms.ColumnHeader columnHeader16;
    }
}