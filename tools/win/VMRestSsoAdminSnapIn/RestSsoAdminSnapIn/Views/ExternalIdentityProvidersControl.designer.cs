/* * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the “License”); you may not * use this file except in compliance with the License.  You may obtain a copy * of the License at http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an “AS IS” BASIS, without * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the * License for the specific language governing permissions and limitations * under the License. */namespace Vmware.Tools.RestSsoAdminSnapIn.Views{
    partial class ExternalIdentityProvidersControl    {        /// <summary>         /// Required designer variable.        /// </summary>        private System.ComponentModel.IContainer components = null;
        /// <summary>         /// Clean up any resources being used.        /// </summary>        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>        protected override void Dispose(bool disposing)        {            if (disposing && (components != null))            {                components.Dispose();            }            base.Dispose(disposing);        }
        #region Component Designer generated code
        /// <summary>         /// Required method for Designer support - do not modify         /// the contents of this method with the code editor.        /// </summary>        private void InitializeComponent()        {
            this.components = new System.ComponentModel.Container();
            this.lstExternalIdentityProviders = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // lstExternalIdentityProviders
            // 
            this.lstExternalIdentityProviders.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader3,
            this.columnHeader2});
            this.lstExternalIdentityProviders.ContextMenuStrip = this.contextMenuStrip1;
            this.lstExternalIdentityProviders.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstExternalIdentityProviders.FullRowSelect = true;
            this.lstExternalIdentityProviders.Location = new System.Drawing.Point(0, 0);
            this.lstExternalIdentityProviders.MultiSelect = false;
            this.lstExternalIdentityProviders.Name = "lstExternalIdentityProviders";
            this.lstExternalIdentityProviders.Size = new System.Drawing.Size(831, 382);
            this.lstExternalIdentityProviders.TabIndex = 1;
            this.lstExternalIdentityProviders.UseCompatibleStateImageBehavior = false;
            this.lstExternalIdentityProviders.View = System.Windows.Forms.View.Details;
            this.lstExternalIdentityProviders.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lstUsers_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Entity ID";
            this.columnHeader1.Width = 495;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Alias";
            this.columnHeader3.Width = 240;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "JIT";
            this.columnHeader2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.columnHeader2.Width = 80;
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.viewToolStripMenuItem,
            this.deleteToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(108, 48);
            this.contextMenuStrip1.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip1_Opening);
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.viewToolStripMenuItem.Text = "View";
            this.viewToolStripMenuItem.Click += new System.EventHandler(this.propertiesToolStripMenuItem_Click);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // ExternalIdentityProvidersControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lstExternalIdentityProviders);
            this.Name = "ExternalIdentityProvidersControl";
            this.Size = new System.Drawing.Size(831, 382);
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion
        private System.Windows.Forms.ListView lstExternalIdentityProviders;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;    }}