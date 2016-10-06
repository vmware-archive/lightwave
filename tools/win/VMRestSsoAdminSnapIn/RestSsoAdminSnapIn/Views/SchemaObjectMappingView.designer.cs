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
    partial class SchemaObjectMappingView
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
            this.dgAttributes = new System.Windows.Forms.DataGridView();
            this.dgContext = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.btnClose = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.txtClassName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.cbObjectId = new System.Windows.Forms.ComboBox();
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributes)).BeginInit();
            this.dgContext.SuspendLayout();
            this.SuspendLayout();
            // 
            // dgAttributes
            // 
            this.dgAttributes.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgAttributes.ContextMenuStrip = this.dgContext;
            this.dgAttributes.Location = new System.Drawing.Point(7, 65);
            this.dgAttributes.MultiSelect = false;
            this.dgAttributes.Name = "dgAttributes";
            this.dgAttributes.Size = new System.Drawing.Size(470, 248);
            this.dgAttributes.TabIndex = 5;
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
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // btnClose
            // 
            this.btnClose.Location = new System.Drawing.Point(402, 319);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 9;
            this.btnClose.Text = "&Close";
            this.btnClose.UseVisualStyleBackColor = true;
            this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
            // 
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(322, 319);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(75, 23);
            this.btnAdd.TabIndex = 8;
            this.btnAdd.Text = "&Add";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // txtClassName
            // 
            this.txtClassName.Location = new System.Drawing.Point(69, 37);
            this.txtClassName.Name = "txtClassName";
            this.txtClassName.Size = new System.Drawing.Size(408, 20);
            this.txtClassName.TabIndex = 7;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(4, 40);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(66, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Class Name:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 14);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Object Id:";
            // 
            // cbObjectId
            // 
            this.cbObjectId.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbObjectId.FormattingEnabled = true;
            this.cbObjectId.Location = new System.Drawing.Point(69, 10);
            this.cbObjectId.Name = "cbObjectId";
            this.cbObjectId.Size = new System.Drawing.Size(147, 21);
            this.cbObjectId.TabIndex = 11;
            this.cbObjectId.SelectedIndexChanged += new System.EventHandler(this.cbObjectId_SelectedIndexChanged);
            // 
            // SchemaObjectMappingView
            // 
            this.AcceptButton = this.btnAdd;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(489, 347);
            this.Controls.Add(this.cbObjectId);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.dgAttributes);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.txtClassName);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SchemaObjectMappingView";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Schema Object Mapping";
            this.Load += new System.EventHandler(this.SchemaObjectMappingView_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributes)).EndInit();
            this.dgContext.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView dgAttributes;
        private System.Windows.Forms.ContextMenuStrip dgContext;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.TextBox txtClassName;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbObjectId;

    }
}