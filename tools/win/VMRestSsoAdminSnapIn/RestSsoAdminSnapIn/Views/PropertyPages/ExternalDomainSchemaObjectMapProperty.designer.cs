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
    partial class ExternalDomainSchemaObjectMapProperty
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
            this.dgContext = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.dgAttributes = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnAdd = new System.Windows.Forms.Button();
            this.btnRemove = new System.Windows.Forms.Button();
            this.dgContext.SuspendLayout();
            this.SuspendLayout();
            // 
            // dgContext
            // 
            this.dgContext.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.deleteToolStripMenuItem});
            this.dgContext.Name = "dgContext";
            this.dgContext.Size = new System.Drawing.Size(108, 26);
            this.dgContext.Opening += new System.ComponentModel.CancelEventHandler(this.dgContext_Opening);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // dgAttributes
            // 
            this.dgAttributes.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.dgAttributes.FullRowSelect = true;
            this.dgAttributes.GridLines = true;
            this.dgAttributes.Location = new System.Drawing.Point(4, 26);
            this.dgAttributes.MultiSelect = false;
            this.dgAttributes.Name = "dgAttributes";
            this.dgAttributes.Size = new System.Drawing.Size(308, 100);
            this.dgAttributes.TabIndex = 1;
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
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(318, 26);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(25, 25);
            this.btnAdd.TabIndex = 2;
            this.btnAdd.Text = "+";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // btnRemove
            // 
            this.btnRemove.Location = new System.Drawing.Point(318, 101);
            this.btnRemove.Name = "btnRemove";
            this.btnRemove.Size = new System.Drawing.Size(25, 25);
            this.btnRemove.TabIndex = 3;
            this.btnRemove.Text = "-";
            this.btnRemove.UseVisualStyleBackColor = true;
            this.btnRemove.Click += new System.EventHandler(this.btnRemove_Click);
            // 
            // ExternalDomainSchemaObjectMapProperty
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.btnRemove);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.dgAttributes);
            this.Name = "ExternalDomainSchemaObjectMapProperty";
            this.Size = new System.Drawing.Size(350, 359);
            this.Load += new System.EventHandler(this.ExternalDomainSchemaObjectMapProperty_Load);
            this.dgContext.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ContextMenuStrip dgContext;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.ListView dgAttributes;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Button btnRemove;
        private System.Windows.Forms.ColumnHeader columnHeader1;






    }
}
