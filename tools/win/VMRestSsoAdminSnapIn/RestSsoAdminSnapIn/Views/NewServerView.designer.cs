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
    partial class NewServerView
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
            this.btnAdd = new System.Windows.Forms.Button();
            this.lnkURLPreview = new System.Windows.Forms.LinkLabel();
            this.label2 = new System.Windows.Forms.Label();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtServer = new System.Windows.Forms.TextBox();
            this.cbIsSsl = new System.Windows.Forms.CheckBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtDefaultTenant = new System.Windows.Forms.TextBox();
            this.addNewServerHelp = new System.Windows.Forms.HelpProvider();
            this.cbSAML = new System.Windows.Forms.CheckBox();
            this.txtStsUrl = new System.Windows.Forms.TextBox();
            this.addNewServerTooltip = new System.Windows.Forms.ToolTip(this.components);
            this.lblStsUrl = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // btnAdd
            // 
            this.btnAdd.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.addNewServerHelp.SetHelpKeyword(this.btnAdd, "AddServer");
            this.addNewServerHelp.SetHelpString(this.btnAdd, "Click to add the server to your MMC SnapIn");
            this.btnAdd.Location = new System.Drawing.Point(299, 184);
            this.btnAdd.Name = "btnAdd";
            this.addNewServerHelp.SetShowHelp(this.btnAdd, true);
            this.btnAdd.Size = new System.Drawing.Size(75, 23);
            this.btnAdd.TabIndex = 23;
            this.btnAdd.Text = "&Add";
            this.addNewServerTooltip.SetToolTip(this.btnAdd, "Click to add the server to your MMC SnapIn");
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // lnkURLPreview
            // 
            this.lnkURLPreview.AutoEllipsis = true;
            this.addNewServerHelp.SetHelpKeyword(this.lnkURLPreview, "ServerURL");
            this.addNewServerHelp.SetHelpString(this.lnkURLPreview, "Click to open the link in the browser");
            this.lnkURLPreview.Location = new System.Drawing.Point(9, 159);
            this.lnkURLPreview.Name = "lnkURLPreview";
            this.addNewServerHelp.SetShowHelp(this.lnkURLPreview, true);
            this.lnkURLPreview.Size = new System.Drawing.Size(364, 18);
            this.lnkURLPreview.TabIndex = 22;
            this.lnkURLPreview.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.addNewServerTooltip.SetToolTip(this.lnkURLPreview, "Click to open the link in the browser");
            this.lnkURLPreview.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lnkURLPreview_LinkClicked);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(86, 49);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 13);
            this.label2.TabIndex = 17;
            this.label2.Text = "STS Port:";
            // 
            // txtPort
            // 
            this.addNewServerHelp.SetHelpKeyword(this.txtPort, "STSPort");
            this.addNewServerHelp.SetHelpString(this.txtPort, "Enter the STS port of the server");
            this.txtPort.Location = new System.Drawing.Point(146, 46);
            this.txtPort.Name = "txtPort";
            this.addNewServerHelp.SetShowHelp(this.txtPort, true);
            this.txtPort.Size = new System.Drawing.Size(228, 20);
            this.txtPort.TabIndex = 16;
            this.txtPort.Text = "443";
            this.addNewServerTooltip.SetToolTip(this.txtPort, "STS port of the server");
            this.txtPort.TextChanged += new System.EventHandler(this.txtPort_TextChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 17);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(134, 13);
            this.label1.TabIndex = 15;
            this.label1.Text = "Server Name / IP Address:";
            // 
            // txtServer
            // 
            this.addNewServerHelp.SetHelpKeyword(this.txtServer, "ServerName");
            this.addNewServerHelp.SetHelpString(this.txtServer, "Enter the Name or IP Address of the Server");
            this.txtServer.Location = new System.Drawing.Point(146, 14);
            this.txtServer.Name = "txtServer";
            this.addNewServerHelp.SetShowHelp(this.txtServer, true);
            this.txtServer.Size = new System.Drawing.Size(228, 20);
            this.txtServer.TabIndex = 14;
            this.addNewServerTooltip.SetToolTip(this.txtServer, "Name or IP Address of the Server");
            this.txtServer.TextChanged += new System.EventHandler(this.txtServer_TextChanged);
            // 
            // cbIsSsl
            // 
            this.cbIsSsl.AutoSize = true;
            this.addNewServerHelp.SetHelpKeyword(this.cbIsSsl, "HTTP");
            this.addNewServerHelp.SetHelpString(this.cbIsSsl, "By default the server is accessed on HTTPS. Click in case you want to set it to H" +
        "TTP.");
            this.cbIsSsl.Location = new System.Drawing.Point(146, 110);
            this.cbIsSsl.Name = "cbIsSsl";
            this.addNewServerHelp.SetShowHelp(this.cbIsSsl, true);
            this.cbIsSsl.Size = new System.Drawing.Size(46, 17);
            this.cbIsSsl.TabIndex = 21;
            this.cbIsSsl.Text = "SSL";
            this.addNewServerTooltip.SetToolTip(this.cbIsSsl, "SSL by default. Click to set it to non-SSL. ");
            this.cbIsSsl.UseVisualStyleBackColor = true;
            this.cbIsSsl.CheckedChanged += new System.EventHandler(this.cbIsHttp_CheckedChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(59, 82);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(81, 13);
            this.label4.TabIndex = 21;
            this.label4.Text = "Default Tenant:";
            // 
            // txtDefaultTenant
            // 
            this.addNewServerHelp.SetHelpKeyword(this.txtDefaultTenant, "DefaultTenant");
            this.addNewServerHelp.SetHelpString(this.txtDefaultTenant, "Enter the default system tenant for the server");
            this.txtDefaultTenant.Location = new System.Drawing.Point(146, 79);
            this.txtDefaultTenant.Name = "txtDefaultTenant";
            this.addNewServerHelp.SetShowHelp(this.txtDefaultTenant, true);
            this.txtDefaultTenant.Size = new System.Drawing.Size(227, 20);
            this.txtDefaultTenant.TabIndex = 20;
            this.addNewServerTooltip.SetToolTip(this.txtDefaultTenant, "Default system tenant for the server");
            this.txtDefaultTenant.TextChanged += new System.EventHandler(this.txtDefaultTenant_TextChanged);
            // 
            // cbSAML
            // 
            this.cbSAML.AutoSize = true;
            this.addNewServerHelp.SetHelpKeyword(this.cbSAML, "SAML");
            this.addNewServerHelp.SetHelpString(this.cbSAML, "Select in case you wish to use SAML protocol against legacy STS endpoint");
            this.cbSAML.Location = new System.Drawing.Point(198, 110);
            this.cbSAML.Name = "cbSAML";
            this.addNewServerHelp.SetShowHelp(this.cbSAML, true);
            this.cbSAML.Size = new System.Drawing.Size(55, 17);
            this.cbSAML.TabIndex = 24;
            this.cbSAML.Text = "SAML";
            this.addNewServerTooltip.SetToolTip(this.cbSAML, "Select in case you wish to use SAML protocol against legacy STS endpoint");
            this.cbSAML.UseVisualStyleBackColor = true;
            this.cbSAML.CheckedChanged += new System.EventHandler(this.cbSAML_CheckedChanged);
            // 
            // txtStsUrl
            // 
            this.addNewServerHelp.SetHelpKeyword(this.txtStsUrl, "DefaultTenant");
            this.addNewServerHelp.SetHelpString(this.txtStsUrl, "Enter the default system tenant for the server");
            this.txtStsUrl.Location = new System.Drawing.Point(144, 133);
            this.txtStsUrl.Name = "txtStsUrl";
            this.addNewServerHelp.SetShowHelp(this.txtStsUrl, true);
            this.txtStsUrl.Size = new System.Drawing.Size(227, 20);
            this.txtStsUrl.TabIndex = 25;
            this.txtStsUrl.Text = "sts/STSService";
            this.addNewServerTooltip.SetToolTip(this.txtStsUrl, "Default system tenant for the server");
            this.txtStsUrl.TextChanged += new System.EventHandler(this.txtStsUrl_TextChanged);
            // 
            // addNewServerTooltip
            // 
            this.addNewServerTooltip.AutomaticDelay = 200;
            this.addNewServerTooltip.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.addNewServerTooltip.ToolTipTitle = "Add New Server";
            // 
            // lblStsUrl
            // 
            this.lblStsUrl.AutoSize = true;
            this.lblStsUrl.Location = new System.Drawing.Point(81, 136);
            this.lblStsUrl.Name = "lblStsUrl";
            this.lblStsUrl.Size = new System.Drawing.Size(56, 13);
            this.lblStsUrl.TabIndex = 26;
            this.lblStsUrl.Text = "STS URL:";
            // 
            // NewServerView
            // 
            this.AcceptButton = this.btnAdd;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 213);
            this.Controls.Add(this.lblStsUrl);
            this.Controls.Add(this.txtStsUrl);
            this.Controls.Add(this.cbSAML);
            this.Controls.Add(this.cbIsSsl);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.lnkURLPreview);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.txtDefaultTenant);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtServer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "NewServerView";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Add New Server";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.LinkLabel lnkURLPreview;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtPort;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtServer;
        private System.Windows.Forms.CheckBox cbIsSsl;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox txtDefaultTenant;
        private System.Windows.Forms.HelpProvider addNewServerHelp;
        private System.Windows.Forms.ToolTip addNewServerTooltip;
        private System.Windows.Forms.CheckBox cbSAML;
        private System.Windows.Forms.Label lblStsUrl;
        private System.Windows.Forms.TextBox txtStsUrl;
    }
}