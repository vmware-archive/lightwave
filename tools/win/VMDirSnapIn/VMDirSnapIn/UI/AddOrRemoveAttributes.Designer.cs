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
    partial class AddOrRemoveAttributes
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
            this.lstNewAttributes = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.lstExistingAttributes = new System.Windows.Forms.ListView();
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnAddSingle = new System.Windows.Forms.Button();
            this.btnAddAll = new System.Windows.Forms.Button();
            this.btnRemoveAll = new System.Windows.Forms.Button();
            this.btnRemoveSingle = new System.Windows.Forms.Button();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lstNewAttributes
            // 
            this.lstNewAttributes.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.lstNewAttributes.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.lstNewAttributes.FullRowSelect = true;
            this.lstNewAttributes.Location = new System.Drawing.Point(12, 12);
            this.lstNewAttributes.Name = "lstNewAttributes";
            this.lstNewAttributes.Size = new System.Drawing.Size(321, 391);
            this.lstNewAttributes.TabIndex = 5;
            this.lstNewAttributes.UseCompatibleStateImageBehavior = false;
            this.lstNewAttributes.View = System.Windows.Forms.View.Details;
            this.lstNewAttributes.VirtualMode = true;
            this.lstNewAttributes.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstNewAttributes_RetrieveVirtualItem);
            this.lstNewAttributes.SelectedIndexChanged += new System.EventHandler(this.lstNewAttributes_SelectedIndexChanged);
            this.lstNewAttributes.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.lstNewAttributes_OnKeyPress);
            this.lstNewAttributes.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lstNewAttributes_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Objectclass";
            this.columnHeader1.Width = 165;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Description";
            this.columnHeader2.Width = 131;
            // 
            // lstExistingAttributes
            // 
            this.lstExistingAttributes.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstExistingAttributes.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3,
            this.columnHeader4});
            this.lstExistingAttributes.FullRowSelect = true;
            this.lstExistingAttributes.Location = new System.Drawing.Point(410, 12);
            this.lstExistingAttributes.Name = "lstExistingAttributes";
            this.lstExistingAttributes.Size = new System.Drawing.Size(321, 391);
            this.lstExistingAttributes.TabIndex = 6;
            this.lstExistingAttributes.UseCompatibleStateImageBehavior = false;
            this.lstExistingAttributes.View = System.Windows.Forms.View.Details;
            this.lstExistingAttributes.VirtualMode = true;
            this.lstExistingAttributes.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstExistingAttributes_RetrieveVirtualItem);
            this.lstExistingAttributes.SelectedIndexChanged += new System.EventHandler(this.lstExistingAttributes_SelectedIndexChanged);
            this.lstExistingAttributes.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lstExistingAttributes_MouseDoubleClick);
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Objectclass";
            this.columnHeader3.Width = 165;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Description";
            this.columnHeader4.Width = 131;
            // 
            // btnAddSingle
            // 
            this.btnAddSingle.Enabled = false;
            this.btnAddSingle.Location = new System.Drawing.Point(339, 136);
            this.btnAddSingle.Name = "btnAddSingle";
            this.btnAddSingle.Size = new System.Drawing.Size(65, 23);
            this.btnAddSingle.TabIndex = 7;
            this.btnAddSingle.Text = ">";
            this.btnAddSingle.UseVisualStyleBackColor = true;
            this.btnAddSingle.Click += new System.EventHandler(this.btnAddSingle_Click);
            // 
            // btnAddAll
            // 
            this.btnAddAll.Enabled = false;
            this.btnAddAll.Location = new System.Drawing.Point(339, 168);
            this.btnAddAll.Name = "btnAddAll";
            this.btnAddAll.Size = new System.Drawing.Size(65, 23);
            this.btnAddAll.TabIndex = 8;
            this.btnAddAll.Text = ">>";
            this.btnAddAll.UseVisualStyleBackColor = true;
            this.btnAddAll.Click += new System.EventHandler(this.btnAddAll_Click);
            // 
            // btnRemoveAll
            // 
            this.btnRemoveAll.Enabled = false;
            this.btnRemoveAll.Location = new System.Drawing.Point(339, 259);
            this.btnRemoveAll.Name = "btnRemoveAll";
            this.btnRemoveAll.Size = new System.Drawing.Size(65, 23);
            this.btnRemoveAll.TabIndex = 10;
            this.btnRemoveAll.Text = "<<";
            this.btnRemoveAll.UseVisualStyleBackColor = true;
            this.btnRemoveAll.Click += new System.EventHandler(this.btnRemoveAll_Click);
            // 
            // btnRemoveSingle
            // 
            this.btnRemoveSingle.Enabled = false;
            this.btnRemoveSingle.Location = new System.Drawing.Point(339, 227);
            this.btnRemoveSingle.Name = "btnRemoveSingle";
            this.btnRemoveSingle.Size = new System.Drawing.Size(65, 23);
            this.btnRemoveSingle.TabIndex = 9;
            this.btnRemoveSingle.Text = "<";
            this.btnRemoveSingle.UseVisualStyleBackColor = true;
            this.btnRemoveSingle.Click += new System.EventHandler(this.btnRemoveSingle_Click);
            // 
            // btnOK
            // 
            this.btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOK.Location = new System.Drawing.Point(574, 412);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 12;
            this.btnOK.Text = "Apply";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(656, 412);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 11;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // AddOrRemoveAttributes
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(743, 447);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnRemoveAll);
            this.Controls.Add(this.btnRemoveSingle);
            this.Controls.Add(this.btnAddAll);
            this.Controls.Add(this.btnAddSingle);
            this.Controls.Add(this.lstExistingAttributes);
            this.Controls.Add(this.lstNewAttributes);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AddOrRemoveAttributes";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Manage attributes";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lstNewAttributes;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ListView lstExistingAttributes;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.Button btnAddSingle;
        private System.Windows.Forms.Button btnAddAll;
        private System.Windows.Forms.Button btnRemoveAll;
        private System.Windows.Forms.Button btnRemoveSingle;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
    }
}