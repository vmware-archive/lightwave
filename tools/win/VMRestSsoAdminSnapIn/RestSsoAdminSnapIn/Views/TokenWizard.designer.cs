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
    partial class TokenWizard
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
            this.btnAdd = new System.Windows.Forms.Button();
            this.lnkURLPreview = new System.Windows.Forms.LinkLabel();
            this.label2 = new System.Windows.Forms.Label();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtServer = new System.Windows.Forms.TextBox();
            this.cbIsSsl = new System.Windows.Forms.CheckBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtDefaultTenant = new System.Windows.Forms.TextBox();
            this.txtDomainName = new System.Windows.Forms.TextBox();
            this.txtUser = new System.Windows.Forms.TextBox();
            this.txtPass = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.txtIdToken = new System.Windows.Forms.TextBox();
            this.lblIdTOken = new System.Windows.Forms.Label();
            this.lblAccessToken = new System.Windows.Forms.Label();
            this.txtAccessToken = new System.Windows.Forms.TextBox();
            this.lblRefreshToken = new System.Windows.Forms.Label();
            this.txtRefreshToken = new System.Windows.Forms.TextBox();
            this.loginTab = new System.Windows.Forms.TabControl();
            this.tabUser = new System.Windows.Forms.TabPage();
            this.tabSolutionUser = new System.Windows.Forms.TabPage();
            this.btnPrivateKey = new System.Windows.Forms.Button();
            this.txtPrivateKey = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtCertificate = new System.Windows.Forms.TextBox();
            this.lblCertificate = new System.Windows.Forms.Label();
            this.btnSelectSertificate = new System.Windows.Forms.Button();
            this.tabGssTicket = new System.Windows.Forms.TabPage();
            this.txtGssPassword = new System.Windows.Forms.TextBox();
            this.txtGssUsername = new System.Windows.Forms.TextBox();
            this.txtGssSpn = new System.Windows.Forms.TextBox();
            this.txtGssDomain = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.tabTokenFile = new System.Windows.Forms.TabPage();
            this.btnTokPKey = new System.Windows.Forms.Button();
            this.txtTokPkey = new System.Windows.Forms.TextBox();
            this.lblTokPKey = new System.Windows.Forms.Label();
            this.txtTokCert = new System.Windows.Forms.TextBox();
            this.lblTokCert = new System.Windows.Forms.Label();
            this.btnTokCert = new System.Windows.Forms.Button();
            this.lblTokFilePath = new System.Windows.Forms.Label();
            this.btnTokFilePath = new System.Windows.Forms.Button();
            this.txtTokenFilePath = new System.Windows.Forms.TextBox();
            this.cbSaml = new System.Windows.Forms.CheckBox();
            this.lblStsUri = new System.Windows.Forms.Label();
            this.txtStsUri = new System.Windows.Forms.TextBox();
            this.lblSamlToken = new System.Windows.Forms.Label();
            this.txtSamlToken = new System.Windows.Forms.TextBox();
            this.loginTab.SuspendLayout();
            this.tabUser.SuspendLayout();
            this.tabSolutionUser.SuspendLayout();
            this.tabGssTicket.SuspendLayout();
            this.tabTokenFile.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnAdd
            // 
            this.btnAdd.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnAdd.Location = new System.Drawing.Point(239, 230);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(132, 23);
            this.btnAdd.TabIndex = 12;
            this.btnAdd.Text = "Acquire Token";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // lnkURLPreview
            // 
            this.lnkURLPreview.AutoEllipsis = true;
            this.lnkURLPreview.Location = new System.Drawing.Point(9, 255);
            this.lnkURLPreview.Name = "lnkURLPreview";
            this.lnkURLPreview.Size = new System.Drawing.Size(364, 18);
            this.lnkURLPreview.TabIndex = 22;
            this.lnkURLPreview.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.lnkURLPreview.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lnkURLPreview_LinkClicked);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(27, 64);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(29, 13);
            this.label2.TabIndex = 17;
            this.label2.Text = "Port:";
            // 
            // txtPort
            // 
            this.txtPort.Location = new System.Drawing.Point(60, 61);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(74, 20);
            this.txtPort.TabIndex = 3;
            this.txtPort.Text = "443";
            this.txtPort.TextChanged += new System.EventHandler(this.txtPort_TextChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 14);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(41, 13);
            this.label1.TabIndex = 15;
            this.label1.Text = "Server:";
            // 
            // txtServer
            // 
            this.txtServer.Location = new System.Drawing.Point(59, 10);
            this.txtServer.Name = "txtServer";
            this.txtServer.Size = new System.Drawing.Size(315, 20);
            this.txtServer.TabIndex = 1;
            this.txtServer.TextChanged += new System.EventHandler(this.txtServer_TextChanged);
            // 
            // cbIsSsl
            // 
            this.cbIsSsl.AutoSize = true;
            this.cbIsSsl.Checked = true;
            this.cbIsSsl.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbIsSsl.Location = new System.Drawing.Point(146, 65);
            this.cbIsSsl.Name = "cbIsSsl";
            this.cbIsSsl.Size = new System.Drawing.Size(46, 17);
            this.cbIsSsl.TabIndex = 4;
            this.cbIsSsl.Text = "SSL";
            this.cbIsSsl.UseVisualStyleBackColor = true;
            this.cbIsSsl.CheckedChanged += new System.EventHandler(this.cbIsHttp_CheckedChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 39);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(44, 13);
            this.label4.TabIndex = 21;
            this.label4.Text = "Tenant:";
            // 
            // txtDefaultTenant
            // 
            this.txtDefaultTenant.Location = new System.Drawing.Point(59, 35);
            this.txtDefaultTenant.Name = "txtDefaultTenant";
            this.txtDefaultTenant.Size = new System.Drawing.Size(314, 20);
            this.txtDefaultTenant.TabIndex = 2;
            this.txtDefaultTenant.Text = "lightwave.local";
            this.txtDefaultTenant.TextChanged += new System.EventHandler(this.txtDefaultTenant_TextChanged);
            // 
            // txtDomainName
            // 
            this.txtDomainName.Location = new System.Drawing.Point(202, 15);
            this.txtDomainName.Name = "txtDomainName";
            this.txtDomainName.Size = new System.Drawing.Size(145, 20);
            this.txtDomainName.TabIndex = 9;
            // 
            // txtUser
            // 
            this.txtUser.Location = new System.Drawing.Point(76, 15);
            this.txtUser.Name = "txtUser";
            this.txtUser.Size = new System.Drawing.Size(100, 20);
            this.txtUser.TabIndex = 8;
            // 
            // txtPass
            // 
            this.txtPass.Location = new System.Drawing.Point(76, 45);
            this.txtPass.Name = "txtPass";
            this.txtPass.PasswordChar = '*';
            this.txtPass.Size = new System.Drawing.Size(271, 20);
            this.txtPass.TabIndex = 10;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(8, 45);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(62, 23);
            this.label5.TabIndex = 26;
            this.label5.Text = "Password:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(8, 13);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(62, 23);
            this.label6.TabIndex = 24;
            this.label6.Text = "User name:";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(180, 15);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(19, 23);
            this.label7.TabIndex = 30;
            this.label7.Text = "@";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // txtIdToken
            // 
            this.txtIdToken.Location = new System.Drawing.Point(15, 298);
            this.txtIdToken.Multiline = true;
            this.txtIdToken.Name = "txtIdToken";
            this.txtIdToken.ReadOnly = true;
            this.txtIdToken.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtIdToken.Size = new System.Drawing.Size(358, 86);
            this.txtIdToken.TabIndex = 13;
            // 
            // lblIdTOken
            // 
            this.lblIdTOken.Location = new System.Drawing.Point(12, 280);
            this.lblIdTOken.Name = "lblIdTOken";
            this.lblIdTOken.Size = new System.Drawing.Size(97, 15);
            this.lblIdTOken.TabIndex = 32;
            this.lblIdTOken.Text = "ID Token:";
            this.lblIdTOken.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblAccessToken
            // 
            this.lblAccessToken.Location = new System.Drawing.Point(11, 393);
            this.lblAccessToken.Name = "lblAccessToken";
            this.lblAccessToken.Size = new System.Drawing.Size(97, 23);
            this.lblAccessToken.TabIndex = 34;
            this.lblAccessToken.Text = "Access Token:";
            this.lblAccessToken.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // txtAccessToken
            // 
            this.txtAccessToken.Location = new System.Drawing.Point(14, 419);
            this.txtAccessToken.Multiline = true;
            this.txtAccessToken.Name = "txtAccessToken";
            this.txtAccessToken.ReadOnly = true;
            this.txtAccessToken.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtAccessToken.Size = new System.Drawing.Size(358, 86);
            this.txtAccessToken.TabIndex = 14;
            // 
            // lblRefreshToken
            // 
            this.lblRefreshToken.Location = new System.Drawing.Point(10, 517);
            this.lblRefreshToken.Name = "lblRefreshToken";
            this.lblRefreshToken.Size = new System.Drawing.Size(97, 23);
            this.lblRefreshToken.TabIndex = 36;
            this.lblRefreshToken.Text = "Refresh Token:";
            this.lblRefreshToken.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // txtRefreshToken
            // 
            this.txtRefreshToken.Location = new System.Drawing.Point(13, 543);
            this.txtRefreshToken.Multiline = true;
            this.txtRefreshToken.Name = "txtRefreshToken";
            this.txtRefreshToken.ReadOnly = true;
            this.txtRefreshToken.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtRefreshToken.Size = new System.Drawing.Size(358, 86);
            this.txtRefreshToken.TabIndex = 15;
            // 
            // loginTab
            // 
            this.loginTab.Controls.Add(this.tabUser);
            this.loginTab.Controls.Add(this.tabSolutionUser);
            this.loginTab.Controls.Add(this.tabGssTicket);
            this.loginTab.Controls.Add(this.tabTokenFile);
            this.loginTab.Location = new System.Drawing.Point(11, 115);
            this.loginTab.Name = "loginTab";
            this.loginTab.SelectedIndex = 0;
            this.loginTab.Size = new System.Drawing.Size(361, 113);
            this.loginTab.TabIndex = 7;
            // 
            // tabUser
            // 
            this.tabUser.BackColor = System.Drawing.SystemColors.Window;
            this.tabUser.Controls.Add(this.label7);
            this.tabUser.Controls.Add(this.txtDomainName);
            this.tabUser.Controls.Add(this.label6);
            this.tabUser.Controls.Add(this.txtUser);
            this.tabUser.Controls.Add(this.label5);
            this.tabUser.Controls.Add(this.txtPass);
            this.tabUser.Location = new System.Drawing.Point(4, 22);
            this.tabUser.Name = "tabUser";
            this.tabUser.Padding = new System.Windows.Forms.Padding(3);
            this.tabUser.Size = new System.Drawing.Size(353, 87);
            this.tabUser.TabIndex = 0;
            this.tabUser.Text = "User Credentials";
            // 
            // tabSolutionUser
            // 
            this.tabSolutionUser.BackColor = System.Drawing.SystemColors.Window;
            this.tabSolutionUser.Controls.Add(this.btnPrivateKey);
            this.tabSolutionUser.Controls.Add(this.txtPrivateKey);
            this.tabSolutionUser.Controls.Add(this.label3);
            this.tabSolutionUser.Controls.Add(this.txtCertificate);
            this.tabSolutionUser.Controls.Add(this.lblCertificate);
            this.tabSolutionUser.Controls.Add(this.btnSelectSertificate);
            this.tabSolutionUser.Location = new System.Drawing.Point(4, 22);
            this.tabSolutionUser.Name = "tabSolutionUser";
            this.tabSolutionUser.Padding = new System.Windows.Forms.Padding(3);
            this.tabSolutionUser.Size = new System.Drawing.Size(353, 87);
            this.tabSolutionUser.TabIndex = 1;
            this.tabSolutionUser.Text = "Solution User Certificate";
            // 
            // btnPrivateKey
            // 
            this.btnPrivateKey.Location = new System.Drawing.Point(327, 25);
            this.btnPrivateKey.Name = "btnPrivateKey";
            this.btnPrivateKey.Size = new System.Drawing.Size(22, 23);
            this.btnPrivateKey.TabIndex = 5;
            this.btnPrivateKey.Text = "...";
            this.btnPrivateKey.UseVisualStyleBackColor = true;
            this.btnPrivateKey.Click += new System.EventHandler(this.btnPrivateKey_Click);
            // 
            // txtPrivateKey
            // 
            this.txtPrivateKey.Location = new System.Drawing.Point(64, 27);
            this.txtPrivateKey.Name = "txtPrivateKey";
            this.txtPrivateKey.Size = new System.Drawing.Size(261, 20);
            this.txtPrivateKey.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(1, 30);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(64, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Private Key:";
            // 
            // txtCertificate
            // 
            this.txtCertificate.Location = new System.Drawing.Point(64, 53);
            this.txtCertificate.Name = "txtCertificate";
            this.txtCertificate.Size = new System.Drawing.Size(261, 20);
            this.txtCertificate.TabIndex = 0;
            // 
            // lblCertificate
            // 
            this.lblCertificate.AutoSize = true;
            this.lblCertificate.Location = new System.Drawing.Point(1, 56);
            this.lblCertificate.Name = "lblCertificate";
            this.lblCertificate.Size = new System.Drawing.Size(57, 13);
            this.lblCertificate.TabIndex = 2;
            this.lblCertificate.Text = "Certificate:";
            // 
            // btnSelectSertificate
            // 
            this.btnSelectSertificate.Location = new System.Drawing.Point(327, 52);
            this.btnSelectSertificate.Name = "btnSelectSertificate";
            this.btnSelectSertificate.Size = new System.Drawing.Size(22, 23);
            this.btnSelectSertificate.TabIndex = 1;
            this.btnSelectSertificate.Text = "...";
            this.btnSelectSertificate.UseVisualStyleBackColor = true;
            this.btnSelectSertificate.Click += new System.EventHandler(this.btnSelectSertificate_Click);
            // 
            // tabGssTicket
            // 
            this.tabGssTicket.BackColor = System.Drawing.SystemColors.Window;
            this.tabGssTicket.Controls.Add(this.txtGssPassword);
            this.tabGssTicket.Controls.Add(this.txtGssUsername);
            this.tabGssTicket.Controls.Add(this.txtGssSpn);
            this.tabGssTicket.Controls.Add(this.txtGssDomain);
            this.tabGssTicket.Controls.Add(this.label13);
            this.tabGssTicket.Controls.Add(this.label10);
            this.tabGssTicket.Controls.Add(this.label11);
            this.tabGssTicket.Controls.Add(this.label12);
            this.tabGssTicket.Location = new System.Drawing.Point(4, 22);
            this.tabGssTicket.Name = "tabGssTicket";
            this.tabGssTicket.Size = new System.Drawing.Size(353, 87);
            this.tabGssTicket.TabIndex = 2;
            this.tabGssTicket.Text = "GSS Ticket";
            // 
            // txtGssPassword
            // 
            this.txtGssPassword.Location = new System.Drawing.Point(63, 44);
            this.txtGssPassword.Name = "txtGssPassword";
            this.txtGssPassword.PasswordChar = '*';
            this.txtGssPassword.Size = new System.Drawing.Size(102, 20);
            this.txtGssPassword.TabIndex = 10;
            // 
            // txtGssUsername
            // 
            this.txtGssUsername.Location = new System.Drawing.Point(63, 14);
            this.txtGssUsername.Name = "txtGssUsername";
            this.txtGssUsername.Size = new System.Drawing.Size(100, 20);
            this.txtGssUsername.TabIndex = 8;
            // 
            // txtGssSpn
            // 
            this.txtGssSpn.Location = new System.Drawing.Point(221, 44);
            this.txtGssSpn.Name = "txtGssSpn";
            this.txtGssSpn.Size = new System.Drawing.Size(125, 20);
            this.txtGssSpn.TabIndex = 11;
            // 
            // txtGssDomain
            // 
            this.txtGssDomain.Location = new System.Drawing.Point(221, 12);
            this.txtGssDomain.Name = "txtGssDomain";
            this.txtGssDomain.Size = new System.Drawing.Size(125, 20);
            this.txtGssDomain.TabIndex = 9;
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(183, 42);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(45, 23);
            this.label13.TabIndex = 37;
            this.label13.Text = "SPN:";
            this.label13.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(170, 12);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(58, 23);
            this.label10.TabIndex = 36;
            this.label10.Text = "Domain:";
            this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(4, 10);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(62, 23);
            this.label11.TabIndex = 31;
            this.label11.Text = "Username:";
            this.label11.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(4, 42);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(62, 23);
            this.label12.TabIndex = 34;
            this.label12.Text = "Password:";
            this.label12.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // tabTokenFile
            // 
            this.tabTokenFile.Controls.Add(this.btnTokPKey);
            this.tabTokenFile.Controls.Add(this.txtTokPkey);
            this.tabTokenFile.Controls.Add(this.lblTokPKey);
            this.tabTokenFile.Controls.Add(this.txtTokCert);
            this.tabTokenFile.Controls.Add(this.lblTokCert);
            this.tabTokenFile.Controls.Add(this.btnTokCert);
            this.tabTokenFile.Controls.Add(this.lblTokFilePath);
            this.tabTokenFile.Controls.Add(this.btnTokFilePath);
            this.tabTokenFile.Controls.Add(this.txtTokenFilePath);
            this.tabTokenFile.Location = new System.Drawing.Point(4, 22);
            this.tabTokenFile.Name = "tabTokenFile";
            this.tabTokenFile.Size = new System.Drawing.Size(353, 87);
            this.tabTokenFile.TabIndex = 3;
            this.tabTokenFile.Text = "Token File";
            this.tabTokenFile.UseVisualStyleBackColor = true;
            // 
            // btnTokPKey
            // 
            this.btnTokPKey.Location = new System.Drawing.Point(327, 6);
            this.btnTokPKey.Name = "btnTokPKey";
            this.btnTokPKey.Size = new System.Drawing.Size(22, 23);
            this.btnTokPKey.TabIndex = 14;
            this.btnTokPKey.Text = "...";
            this.btnTokPKey.UseVisualStyleBackColor = true;
            this.btnTokPKey.Click += new System.EventHandler(this.btnTokPKey_Click);
            // 
            // txtTokPkey
            // 
            this.txtTokPkey.Location = new System.Drawing.Point(64, 8);
            this.txtTokPkey.Name = "txtTokPkey";
            this.txtTokPkey.Size = new System.Drawing.Size(261, 20);
            this.txtTokPkey.TabIndex = 12;
            // 
            // lblTokPKey
            // 
            this.lblTokPKey.AutoSize = true;
            this.lblTokPKey.Location = new System.Drawing.Point(1, 11);
            this.lblTokPKey.Name = "lblTokPKey";
            this.lblTokPKey.Size = new System.Drawing.Size(64, 13);
            this.lblTokPKey.TabIndex = 13;
            this.lblTokPKey.Text = "Private Key:";
            // 
            // txtTokCert
            // 
            this.txtTokCert.Location = new System.Drawing.Point(64, 34);
            this.txtTokCert.Name = "txtTokCert";
            this.txtTokCert.Size = new System.Drawing.Size(261, 20);
            this.txtTokCert.TabIndex = 9;
            // 
            // lblTokCert
            // 
            this.lblTokCert.AutoSize = true;
            this.lblTokCert.Location = new System.Drawing.Point(5, 37);
            this.lblTokCert.Name = "lblTokCert";
            this.lblTokCert.Size = new System.Drawing.Size(57, 13);
            this.lblTokCert.TabIndex = 11;
            this.lblTokCert.Text = "Certificate:";
            // 
            // btnTokCert
            // 
            this.btnTokCert.Location = new System.Drawing.Point(327, 33);
            this.btnTokCert.Name = "btnTokCert";
            this.btnTokCert.Size = new System.Drawing.Size(22, 23);
            this.btnTokCert.TabIndex = 10;
            this.btnTokCert.Text = "...";
            this.btnTokCert.UseVisualStyleBackColor = true;
            this.btnTokCert.Click += new System.EventHandler(this.button3_Click);
            // 
            // lblTokFilePath
            // 
            this.lblTokFilePath.AutoSize = true;
            this.lblTokFilePath.Location = new System.Drawing.Point(12, 62);
            this.lblTokFilePath.Name = "lblTokFilePath";
            this.lblTokFilePath.Size = new System.Drawing.Size(41, 13);
            this.lblTokFilePath.TabIndex = 8;
            this.lblTokFilePath.Text = "Token:";
            // 
            // btnTokFilePath
            // 
            this.btnTokFilePath.Location = new System.Drawing.Point(327, 58);
            this.btnTokFilePath.Name = "btnTokFilePath";
            this.btnTokFilePath.Size = new System.Drawing.Size(22, 23);
            this.btnTokFilePath.TabIndex = 7;
            this.btnTokFilePath.Text = "...";
            this.btnTokFilePath.UseVisualStyleBackColor = true;
            this.btnTokFilePath.Click += new System.EventHandler(this.button1_Click);
            // 
            // txtTokenFilePath
            // 
            this.txtTokenFilePath.Location = new System.Drawing.Point(64, 59);
            this.txtTokenFilePath.Name = "txtTokenFilePath";
            this.txtTokenFilePath.Size = new System.Drawing.Size(260, 20);
            this.txtTokenFilePath.TabIndex = 6;
            // 
            // cbSaml
            // 
            this.cbSaml.AutoSize = true;
            this.cbSaml.Location = new System.Drawing.Point(201, 65);
            this.cbSaml.Name = "cbSaml";
            this.cbSaml.Size = new System.Drawing.Size(55, 17);
            this.cbSaml.TabIndex = 5;
            this.cbSaml.Text = "SAML";
            this.cbSaml.UseVisualStyleBackColor = true;
            this.cbSaml.CheckedChanged += new System.EventHandler(this.cbSaml_CheckedChanged);
            // 
            // lblStsUri
            // 
            this.lblStsUri.AutoSize = true;
            this.lblStsUri.Location = new System.Drawing.Point(5, 92);
            this.lblStsUri.Name = "lblStsUri";
            this.lblStsUri.Size = new System.Drawing.Size(53, 13);
            this.lblStsUri.TabIndex = 39;
            this.lblStsUri.Text = "STS URI:";
            // 
            // txtStsUri
            // 
            this.txtStsUri.Location = new System.Drawing.Point(59, 87);
            this.txtStsUri.Name = "txtStsUri";
            this.txtStsUri.Size = new System.Drawing.Size(315, 20);
            this.txtStsUri.TabIndex = 6;
            this.txtStsUri.Text = "sts/STSService";
            this.txtStsUri.TextChanged += new System.EventHandler(this.txtStsUri_TextChanged);
            // 
            // lblSamlToken
            // 
            this.lblSamlToken.Location = new System.Drawing.Point(10, 280);
            this.lblSamlToken.Name = "lblSamlToken";
            this.lblSamlToken.Size = new System.Drawing.Size(97, 19);
            this.lblSamlToken.TabIndex = 40;
            this.lblSamlToken.Text = "SAML Token:";
            this.lblSamlToken.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // txtSamlToken
            // 
            this.txtSamlToken.Location = new System.Drawing.Point(10, 300);
            this.txtSamlToken.Multiline = true;
            this.txtSamlToken.Name = "txtSamlToken";
            this.txtSamlToken.ReadOnly = true;
            this.txtSamlToken.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtSamlToken.Size = new System.Drawing.Size(358, 329);
            this.txtSamlToken.TabIndex = 41;
            // 
            // TokenWizard
            // 
            this.AcceptButton = this.btnAdd;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(381, 635);
            this.Controls.Add(this.txtSamlToken);
            this.Controls.Add(this.lblSamlToken);
            this.Controls.Add(this.txtStsUri);
            this.Controls.Add(this.lblStsUri);
            this.Controls.Add(this.cbSaml);
            this.Controls.Add(this.loginTab);
            this.Controls.Add(this.lblRefreshToken);
            this.Controls.Add(this.txtRefreshToken);
            this.Controls.Add(this.lblAccessToken);
            this.Controls.Add(this.txtAccessToken);
            this.Controls.Add(this.lblIdTOken);
            this.Controls.Add(this.txtIdToken);
            this.Controls.Add(this.cbIsSsl);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.lnkURLPreview);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.txtDefaultTenant);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtServer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "TokenWizard";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Diagnostics - Token";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.TokenWizard_FormClosing);
            this.Load += new System.EventHandler(this.TokenWizard_Load);
            this.loginTab.ResumeLayout(false);
            this.tabUser.ResumeLayout(false);
            this.tabUser.PerformLayout();
            this.tabSolutionUser.ResumeLayout(false);
            this.tabSolutionUser.PerformLayout();
            this.tabGssTicket.ResumeLayout(false);
            this.tabGssTicket.PerformLayout();
            this.tabTokenFile.ResumeLayout(false);
            this.tabTokenFile.PerformLayout();
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
        private System.Windows.Forms.TextBox txtDomainName;
        private System.Windows.Forms.TextBox txtUser;
        private System.Windows.Forms.TextBox txtPass;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox txtIdToken;
        private System.Windows.Forms.Label lblIdTOken;
        private System.Windows.Forms.Label lblAccessToken;
        private System.Windows.Forms.TextBox txtAccessToken;
        private System.Windows.Forms.Label lblRefreshToken;
        private System.Windows.Forms.TextBox txtRefreshToken;
        private System.Windows.Forms.TabControl loginTab;
        private System.Windows.Forms.TabPage tabUser;
        private System.Windows.Forms.TabPage tabSolutionUser;
        private System.Windows.Forms.TabPage tabGssTicket;
        private System.Windows.Forms.Label lblCertificate;
        private System.Windows.Forms.Button btnSelectSertificate;
        private System.Windows.Forms.TextBox txtCertificate;
        private System.Windows.Forms.TextBox txtGssSpn;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox txtGssDomain;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.TextBox txtGssUsername;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox txtGssPassword;
        private System.Windows.Forms.CheckBox cbSaml;
        private System.Windows.Forms.Label lblStsUri;
        private System.Windows.Forms.TextBox txtStsUri;
        private System.Windows.Forms.Label lblSamlToken;
        private System.Windows.Forms.TextBox txtSamlToken;
        private System.Windows.Forms.TabPage tabTokenFile;
        private System.Windows.Forms.Label lblTokFilePath;
        private System.Windows.Forms.Button btnTokFilePath;
        private System.Windows.Forms.TextBox txtTokenFilePath;
        private System.Windows.Forms.Label lblTokCert;
        private System.Windows.Forms.Button btnTokCert;
        private System.Windows.Forms.TextBox txtTokCert;
        private System.Windows.Forms.Button btnPrivateKey;
        private System.Windows.Forms.TextBox txtPrivateKey;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button btnTokPKey;
        private System.Windows.Forms.TextBox txtTokPkey;
        private System.Windows.Forms.Label lblTokPKey;
    }
}