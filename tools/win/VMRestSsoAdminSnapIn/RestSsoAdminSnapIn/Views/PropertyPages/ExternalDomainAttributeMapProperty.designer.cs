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

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    partial class ExternalDomainAttributeMapProperty
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
            this.dgAttributeMap = new System.Windows.Forms.DataGridView();
            this.dgContext = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributeMap)).BeginInit();
            this.dgContext.SuspendLayout();
            this.SuspendLayout();
            // 
            // dgAttributeMap
            // 
            this.dgAttributeMap.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgAttributeMap.ContextMenuStrip = this.dgContext;
            this.dgAttributeMap.Location = new System.Drawing.Point(4, 34);
            this.dgAttributeMap.MultiSelect = false;
            this.dgAttributeMap.Name = "dgAttributeMap";
            this.dgAttributeMap.Size = new System.Drawing.Size(326, 280);
            this.dgAttributeMap.TabIndex = 0;
            // 
            // dgContext
            // 
            this.dgContext.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.deleteToolStripMenuItem});
            this.dgContext.Name = "dgContext";
            this.dgContext.Size = new System.Drawing.Size(108, 26);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // ExternalDomainAttributeMapProperty
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.dgAttributeMap);
            this.Name = "ExternalDomainAttributeMapProperty";
            this.Size = new System.Drawing.Size(350, 359);
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributeMap)).EndInit();
            this.dgContext.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGridView dgAttributeMap;
        private System.Windows.Forms.ContextMenuStrip dgContext;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;






    }
}
