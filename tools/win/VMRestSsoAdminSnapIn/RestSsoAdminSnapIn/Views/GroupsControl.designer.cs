/* * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the “License”); you may not * use this file except in compliance with the License.  You may obtain a copy * of the License at http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an “AS IS” BASIS, without * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the * License for the specific language governing permissions and limitations * under the License. */ namespace Vmware.Tools.RestSsoAdminSnapIn.Views{    partial class GroupsControl    {        /// <summary>         /// Required designer variable.        /// </summary>        private System.ComponentModel.IContainer components = null;
        /// <summary>         /// Clean up any resources being used.        /// </summary>        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>        protected override void Dispose(bool disposing)        {            if (disposing && (components != null))            {                components.Dispose();            }            base.Dispose(disposing);        }
        #region Component Designer generated code
        /// <summary>         /// Required method for Designer support - do not modify         /// the contents of this method with the code editor.        /// </summary>        private void InitializeComponent()        {            this.components = new System.ComponentModel.Container();
            this.lstGroups = new System.Windows.Forms.ListView();
            this.columnHeader15 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader17 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addToGroupToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.renameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.propertiesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // lstGroups
            // 
            this.lstGroups.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader15,
            this.columnHeader17});
            this.lstGroups.ContextMenuStrip = this.contextMenuStrip1;
            this.lstGroups.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstGroups.FullRowSelect = true;
            this.lstGroups.Location = new System.Drawing.Point(0, 0);
            this.lstGroups.MultiSelect = false;
            this.lstGroups.Name = "lstGroups";
            this.lstGroups.Size = new System.Drawing.Size(715, 279);
            this.lstGroups.TabIndex = 3;
            this.lstGroups.UseCompatibleStateImageBehavior = false;
            this.lstGroups.View = System.Windows.Forms.View.Details;
            this.lstGroups.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lstGroups_MouseDoubleClick);
            // 
            // columnHeader15
            // 
            this.columnHeader15.Text = "Principal Name";
            this.columnHeader15.Width = 297;
            // 
            // columnHeader17
            // 
            this.columnHeader17.Text = "Description";
            this.columnHeader17.Width = 407;
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addToGroupToolStripMenuItem,
            this.toolStripSeparator1,
            this.deleteToolStripMenuItem,
            this.renameToolStripMenuItem,
            this.propertiesToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(156, 98);
            this.contextMenuStrip1.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip1_Opening);
            // 
            // addToGroupToolStripMenuItem
            // 
            this.addToGroupToolStripMenuItem.Name = "addToGroupToolStripMenuItem";
            this.addToGroupToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.addToGroupToolStripMenuItem.Text = "Add to Group...";
            this.addToGroupToolStripMenuItem.Click += new System.EventHandler(this.addToGroupToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(152, 6);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // renameToolStripMenuItem
            // 
            this.renameToolStripMenuItem.Name = "renameToolStripMenuItem";
            this.renameToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.renameToolStripMenuItem.Text = "Rename";
            // 
            // propertiesToolStripMenuItem
            // 
            this.propertiesToolStripMenuItem.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
            this.propertiesToolStripMenuItem.Name = "propertiesToolStripMenuItem";
            this.propertiesToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.propertiesToolStripMenuItem.Text = "Properties";
            this.propertiesToolStripMenuItem.Click += new System.EventHandler(this.propertiesToolStripMenuItem_Click);
            // 
            // GroupsControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lstGroups);
            this.Name = "GroupsControl";
            this.Size = new System.Drawing.Size(715, 279);
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion
        private System.Windows.Forms.ListView lstGroups;
        private System.Windows.Forms.ColumnHeader columnHeader15;        private System.Windows.Forms.ColumnHeader columnHeader17;        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;        private System.Windows.Forms.ToolStripMenuItem addToGroupToolStripMenuItem;        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;        private System.Windows.Forms.ToolStripMenuItem renameToolStripMenuItem;        private System.Windows.Forms.ToolStripMenuItem propertiesToolStripMenuItem;
    }}