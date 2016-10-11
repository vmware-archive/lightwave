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

namespace VMDirSchemaSnapIn.UI
{
    partial class ViewDiffWindow
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
            this.ViewDiffGrid = new System.Windows.Forms.DataGridView();
            this.BaseNode = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Current = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.ViewDiffGrid)).BeginInit();
            this.SuspendLayout();
            // 
            // ViewDiffGrid
            // 
            this.ViewDiffGrid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.ViewDiffGrid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.BaseNode,
            this.Current});
            this.ViewDiffGrid.Location = new System.Drawing.Point(32, 31);
            this.ViewDiffGrid.Name = "ViewDiffGrid";
            this.ViewDiffGrid.Size = new System.Drawing.Size(387, 269);
            this.ViewDiffGrid.TabIndex = 0;
            this.ViewDiffGrid.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DiffDoubleClicked);
            // 
            // BaseNode
            // 
            this.BaseNode.HeaderText = "Base";
            this.BaseNode.Name = "BaseNode";
            this.BaseNode.ReadOnly = true;
            // 
            // Current
            // 
            this.Current.HeaderText = "Current";
            this.Current.Name = "Current";
            this.Current.ReadOnly = true;
            // 
            // ViewDiffWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(442, 325);
            this.Controls.Add(this.ViewDiffGrid);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "ViewDiffWindow";
            this.Text = "ViewDiffWindow";
            ((System.ComponentModel.ISupportInitialize)(this.ViewDiffGrid)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGridView ViewDiffGrid;
        private System.Windows.Forms.DataGridViewTextBoxColumn BaseNode;
        private System.Windows.Forms.DataGridViewTextBoxColumn Current;


    }
}