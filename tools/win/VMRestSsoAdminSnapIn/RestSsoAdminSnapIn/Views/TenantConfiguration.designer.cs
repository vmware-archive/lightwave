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
    partial class TenantConfiguration
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
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnApply = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.txtBrandLogonContent = new System.Windows.Forms.TextBox();
            this.txtBrandName = new System.Windows.Forms.TextBox();
            this.txtAuthenticationCrlDistributionPointOverride = new System.Windows.Forms.TextBox();
            this.cbAuthenticationCrlDistributionPointUsage = new System.Windows.Forms.CheckBox();
            this.txtAuthenticationOcspUrlOverride = new System.Windows.Forms.TextBox();
            this.cbAuthenticationFailoverToCrl = new System.Windows.Forms.CheckBox();
            this.cbAuthenticationOcsp = new System.Windows.Forms.CheckBox();
            this.cbAuthenticationRevocationCheck = new System.Windows.Forms.CheckBox();
            this.nudLockoutAutounlockIntervalSecs = new System.Windows.Forms.NumericUpDown();
            this.nudLockoutMaxFailedAttempts = new System.Windows.Forms.NumericUpDown();
            this.nudLockoutFailedAttemptIntervalSecs = new System.Windows.Forms.NumericUpDown();
            this.txtProviderDefault = new System.Windows.Forms.TextBox();
            this.nudPasswordProhibitedPreviousPasswordCount = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordLifetimeDays = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMinUppercaseCount = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMinSpecialCharacterCount = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMinNumericCount = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMinLowercaseCount = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMinLength = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMinAlphabeticCount = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMaxLength = new System.Windows.Forms.NumericUpDown();
            this.nudPasswordMaxIdenticalAdjacentChars = new System.Windows.Forms.NumericUpDown();
            this.txtPasswordDescription = new System.Windows.Forms.TextBox();
            this.nudTokenHokMaxRefreshLifetimeMillis = new System.Windows.Forms.NumericUpDown();
            this.nudTokenBearerMaxRefreshLifetimeMillis = new System.Windows.Forms.NumericUpDown();
            this.nudTokenRenewCount = new System.Windows.Forms.NumericUpDown();
            this.nudTokenHokMaxLifetimeMillis = new System.Windows.Forms.NumericUpDown();
            this.nudTokenBearerMaxLifetimeMillis = new System.Windows.Forms.NumericUpDown();
            this.nudTokenDelegationCount = new System.Windows.Forms.NumericUpDown();
            this.nudTokenClockTolerenceMillis = new System.Windows.Forms.NumericUpDown();
            this.btnRemoveCert = new System.Windows.Forms.Button();
            this.btnAddCert = new System.Windows.Forms.Button();
            this.btnAuthenticationRemoveOid = new System.Windows.Forms.Button();
            this.btnAuthenticationAddOid = new System.Windows.Forms.Button();
            this.txtLogonTitle = new System.Windows.Forms.TextBox();
            this.chkDisableBanner = new System.Windows.Forms.CheckBox();
            this.chkLogonBannerCheckbox = new System.Windows.Forms.CheckBox();
            this.chkAuthPolicyPassword = new System.Windows.Forms.CheckBox();
            this.chkAuthPolicyWindows = new System.Windows.Forms.CheckBox();
            this.chkAuthPolicyCertificate = new System.Windows.Forms.CheckBox();
            this.button1 = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label31 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.lstAuthenticationCertificatePolicyOids = new System.Windows.Forms.ListView();
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.lstAuthenticationCertificates = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label30 = new System.Windows.Forms.Label();
            this.label29 = new System.Windows.Forms.Label();
            this.groupBox9 = new System.Windows.Forms.GroupBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label28 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.txtLockoutDescription = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.label27 = new System.Windows.Forms.Label();
            this.label26 = new System.Windows.Forms.Label();
            this.label25 = new System.Windows.Forms.Label();
            this.label24 = new System.Windows.Forms.Label();
            this.label23 = new System.Windows.Forms.Label();
            this.label22 = new System.Windows.Forms.Label();
            this.label21 = new System.Windows.Forms.Label();
            this.label20 = new System.Windows.Forms.Label();
            this.label19 = new System.Windows.Forms.Label();
            this.label18 = new System.Windows.Forms.Label();
            this.label17 = new System.Windows.Forms.Label();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.groupBox8 = new System.Windows.Forms.GroupBox();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.label16 = new System.Windows.Forms.Label();
            this.label15 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.txtProviderAlias = new System.Windows.Forms.TextBox();
            this.label32 = new System.Windows.Forms.Label();
            this.cbProviderSelection = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.nudLockoutAutounlockIntervalSecs)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudLockoutMaxFailedAttempts)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudLockoutFailedAttemptIntervalSecs)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordProhibitedPreviousPasswordCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordLifetimeDays)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinUppercaseCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinSpecialCharacterCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinNumericCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinLowercaseCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinLength)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinAlphabeticCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMaxLength)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMaxIdenticalAdjacentChars)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenHokMaxRefreshLifetimeMillis)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenBearerMaxRefreshLifetimeMillis)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenRenewCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenHokMaxLifetimeMillis)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenBearerMaxLifetimeMillis)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenDelegationCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenClockTolerenceMillis)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(755, 528);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(74, 23);
            this.btnCancel.TabIndex = 40;
            this.btnCancel.Text = "&Close";
            this.toolTip1.SetToolTip(this.btnCancel, "Closes the Tenant configuration window");
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnApply
            // 
            this.btnApply.Location = new System.Drawing.Point(675, 530);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new System.Drawing.Size(74, 23);
            this.btnApply.TabIndex = 39;
            this.btnApply.Text = "&Update";
            this.toolTip1.SetToolTip(this.btnApply, "Updates the Tenant configuration");
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Click += new System.EventHandler(this.btnApply_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 2000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.ReshowDelay = 100;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.toolTip1.ToolTipTitle = "HTTP Transport Log";
            // 
            // txtBrandLogonContent
            // 
            this.txtBrandLogonContent.Location = new System.Drawing.Point(52, 72);
            this.txtBrandLogonContent.Multiline = true;
            this.txtBrandLogonContent.Name = "txtBrandLogonContent";
            this.txtBrandLogonContent.Size = new System.Drawing.Size(197, 64);
            this.txtBrandLogonContent.TabIndex = 38;
            this.toolTip1.SetToolTip(this.txtBrandLogonContent, "Enter the logon banner of the brand policy");
            // 
            // txtBrandName
            // 
            this.txtBrandName.Location = new System.Drawing.Point(52, 24);
            this.txtBrandName.Name = "txtBrandName";
            this.txtBrandName.Size = new System.Drawing.Size(197, 20);
            this.txtBrandName.TabIndex = 37;
            this.toolTip1.SetToolTip(this.txtBrandName, "Enter the name of the brand policy");
            // 
            // txtAuthenticationCrlDistributionPointOverride
            // 
            this.txtAuthenticationCrlDistributionPointOverride.Location = new System.Drawing.Point(161, 225);
            this.txtAuthenticationCrlDistributionPointOverride.Name = "txtAuthenticationCrlDistributionPointOverride";
            this.txtAuthenticationCrlDistributionPointOverride.Size = new System.Drawing.Size(113, 20);
            this.txtAuthenticationCrlDistributionPointOverride.TabIndex = 20;
            this.toolTip1.SetToolTip(this.txtAuthenticationCrlDistributionPointOverride, "Enter the CRL distribution point override");
            // 
            // cbAuthenticationCrlDistributionPointUsage
            // 
            this.cbAuthenticationCrlDistributionPointUsage.AutoSize = true;
            this.cbAuthenticationCrlDistributionPointUsage.Location = new System.Drawing.Point(116, 168);
            this.cbAuthenticationCrlDistributionPointUsage.Name = "cbAuthenticationCrlDistributionPointUsage";
            this.cbAuthenticationCrlDistributionPointUsage.Size = new System.Drawing.Size(158, 17);
            this.cbAuthenticationCrlDistributionPointUsage.TabIndex = 18;
            this.cbAuthenticationCrlDistributionPointUsage.Text = "CRL distribution point usage";
            this.toolTip1.SetToolTip(this.cbAuthenticationCrlDistributionPointUsage, "Sets the CRL distribution point usage");
            this.cbAuthenticationCrlDistributionPointUsage.UseVisualStyleBackColor = true;
            // 
            // txtAuthenticationOcspUrlOverride
            // 
            this.txtAuthenticationOcspUrlOverride.Location = new System.Drawing.Point(117, 199);
            this.txtAuthenticationOcspUrlOverride.Name = "txtAuthenticationOcspUrlOverride";
            this.txtAuthenticationOcspUrlOverride.Size = new System.Drawing.Size(157, 20);
            this.txtAuthenticationOcspUrlOverride.TabIndex = 19;
            this.toolTip1.SetToolTip(this.txtAuthenticationOcspUrlOverride, "Enter the OCSP URL override");
            // 
            // cbAuthenticationFailoverToCrl
            // 
            this.cbAuthenticationFailoverToCrl.AutoSize = true;
            this.cbAuthenticationFailoverToCrl.Location = new System.Drawing.Point(11, 145);
            this.cbAuthenticationFailoverToCrl.Name = "cbAuthenticationFailoverToCrl";
            this.cbAuthenticationFailoverToCrl.Size = new System.Drawing.Size(99, 17);
            this.cbAuthenticationFailoverToCrl.TabIndex = 15;
            this.cbAuthenticationFailoverToCrl.Text = "Failover to CRL";
            this.toolTip1.SetToolTip(this.cbAuthenticationFailoverToCrl, "Sets the Failover to CRL");
            this.cbAuthenticationFailoverToCrl.UseVisualStyleBackColor = true;
            // 
            // cbAuthenticationOcsp
            // 
            this.cbAuthenticationOcsp.AutoSize = true;
            this.cbAuthenticationOcsp.Location = new System.Drawing.Point(12, 168);
            this.cbAuthenticationOcsp.Name = "cbAuthenticationOcsp";
            this.cbAuthenticationOcsp.Size = new System.Drawing.Size(55, 17);
            this.cbAuthenticationOcsp.TabIndex = 17;
            this.cbAuthenticationOcsp.Text = "OCSP";
            this.toolTip1.SetToolTip(this.cbAuthenticationOcsp, "Sets the OCSP");
            this.cbAuthenticationOcsp.UseVisualStyleBackColor = true;
            // 
            // cbAuthenticationRevocationCheck
            // 
            this.cbAuthenticationRevocationCheck.AutoSize = true;
            this.cbAuthenticationRevocationCheck.Location = new System.Drawing.Point(116, 145);
            this.cbAuthenticationRevocationCheck.Name = "cbAuthenticationRevocationCheck";
            this.cbAuthenticationRevocationCheck.Size = new System.Drawing.Size(115, 17);
            this.cbAuthenticationRevocationCheck.TabIndex = 16;
            this.cbAuthenticationRevocationCheck.Text = "Revocation Check";
            this.toolTip1.SetToolTip(this.cbAuthenticationRevocationCheck, "Sets the revocation check");
            this.cbAuthenticationRevocationCheck.UseVisualStyleBackColor = true;
            // 
            // nudLockoutAutounlockIntervalSecs
            // 
            this.nudLockoutAutounlockIntervalSecs.Location = new System.Drawing.Point(153, 109);
            this.nudLockoutAutounlockIntervalSecs.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudLockoutAutounlockIntervalSecs.Name = "nudLockoutAutounlockIntervalSecs";
            this.nudLockoutAutounlockIntervalSecs.Size = new System.Drawing.Size(86, 20);
            this.nudLockoutAutounlockIntervalSecs.TabIndex = 4;
            this.toolTip1.SetToolTip(this.nudLockoutAutounlockIntervalSecs, "Enter auto-unlock interval in seconds");
            // 
            // nudLockoutMaxFailedAttempts
            // 
            this.nudLockoutMaxFailedAttempts.Location = new System.Drawing.Point(154, 83);
            this.nudLockoutMaxFailedAttempts.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudLockoutMaxFailedAttempts.Name = "nudLockoutMaxFailedAttempts";
            this.nudLockoutMaxFailedAttempts.Size = new System.Drawing.Size(86, 20);
            this.nudLockoutMaxFailedAttempts.TabIndex = 3;
            this.toolTip1.SetToolTip(this.nudLockoutMaxFailedAttempts, "Enter maximum failed attempts");
            // 
            // nudLockoutFailedAttemptIntervalSecs
            // 
            this.nudLockoutFailedAttemptIntervalSecs.Location = new System.Drawing.Point(153, 56);
            this.nudLockoutFailedAttemptIntervalSecs.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudLockoutFailedAttemptIntervalSecs.Name = "nudLockoutFailedAttemptIntervalSecs";
            this.nudLockoutFailedAttemptIntervalSecs.Size = new System.Drawing.Size(86, 20);
            this.nudLockoutFailedAttemptIntervalSecs.TabIndex = 2;
            this.toolTip1.SetToolTip(this.nudLockoutFailedAttemptIntervalSecs, "Enter the failed attempt interval in seconds");
            // 
            // txtProviderDefault
            // 
            this.txtProviderDefault.Location = new System.Drawing.Point(52, 20);
            this.txtProviderDefault.Name = "txtProviderDefault";
            this.txtProviderDefault.Size = new System.Drawing.Size(181, 20);
            this.txtProviderDefault.TabIndex = 36;
            this.toolTip1.SetToolTip(this.txtProviderDefault, "Enter the default provider");
            // 
            // nudPasswordProhibitedPreviousPasswordCount
            // 
            this.nudPasswordProhibitedPreviousPasswordCount.Location = new System.Drawing.Point(163, 284);
            this.nudPasswordProhibitedPreviousPasswordCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordProhibitedPreviousPasswordCount.Name = "nudPasswordProhibitedPreviousPasswordCount";
            this.nudPasswordProhibitedPreviousPasswordCount.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordProhibitedPreviousPasswordCount.TabIndex = 35;
            this.toolTip1.SetToolTip(this.nudPasswordProhibitedPreviousPasswordCount, "Enter the prohibited previous password count");
            // 
            // nudPasswordLifetimeDays
            // 
            this.nudPasswordLifetimeDays.Location = new System.Drawing.Point(163, 256);
            this.nudPasswordLifetimeDays.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordLifetimeDays.Name = "nudPasswordLifetimeDays";
            this.nudPasswordLifetimeDays.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordLifetimeDays.TabIndex = 34;
            this.toolTip1.SetToolTip(this.nudPasswordLifetimeDays, "Enter the lifetime of the password in days");
            // 
            // nudPasswordMinUppercaseCount
            // 
            this.nudPasswordMinUppercaseCount.Location = new System.Drawing.Point(163, 230);
            this.nudPasswordMinUppercaseCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMinUppercaseCount.Name = "nudPasswordMinUppercaseCount";
            this.nudPasswordMinUppercaseCount.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMinUppercaseCount.TabIndex = 33;
            this.toolTip1.SetToolTip(this.nudPasswordMinUppercaseCount, "Enter the minimum upper case count allowed in the password");
            // 
            // nudPasswordMinSpecialCharacterCount
            // 
            this.nudPasswordMinSpecialCharacterCount.Location = new System.Drawing.Point(163, 203);
            this.nudPasswordMinSpecialCharacterCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMinSpecialCharacterCount.Name = "nudPasswordMinSpecialCharacterCount";
            this.nudPasswordMinSpecialCharacterCount.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMinSpecialCharacterCount.TabIndex = 32;
            this.toolTip1.SetToolTip(this.nudPasswordMinSpecialCharacterCount, "Enter the minimum special character count allowed in the password");
            // 
            // nudPasswordMinNumericCount
            // 
            this.nudPasswordMinNumericCount.Location = new System.Drawing.Point(163, 177);
            this.nudPasswordMinNumericCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMinNumericCount.Name = "nudPasswordMinNumericCount";
            this.nudPasswordMinNumericCount.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMinNumericCount.TabIndex = 31;
            this.toolTip1.SetToolTip(this.nudPasswordMinNumericCount, "Enter the minimum numeric count allowed in the password");
            // 
            // nudPasswordMinLowercaseCount
            // 
            this.nudPasswordMinLowercaseCount.Location = new System.Drawing.Point(163, 151);
            this.nudPasswordMinLowercaseCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMinLowercaseCount.Name = "nudPasswordMinLowercaseCount";
            this.nudPasswordMinLowercaseCount.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMinLowercaseCount.TabIndex = 30;
            this.toolTip1.SetToolTip(this.nudPasswordMinLowercaseCount, "Enter the minimum lowercase count allowed in the password");
            // 
            // nudPasswordMinLength
            // 
            this.nudPasswordMinLength.Location = new System.Drawing.Point(163, 125);
            this.nudPasswordMinLength.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMinLength.Name = "nudPasswordMinLength";
            this.nudPasswordMinLength.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMinLength.TabIndex = 29;
            this.toolTip1.SetToolTip(this.nudPasswordMinLength, "Enter the minimum length of the password");
            // 
            // nudPasswordMinAlphabeticCount
            // 
            this.nudPasswordMinAlphabeticCount.Location = new System.Drawing.Point(163, 99);
            this.nudPasswordMinAlphabeticCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMinAlphabeticCount.Name = "nudPasswordMinAlphabeticCount";
            this.nudPasswordMinAlphabeticCount.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMinAlphabeticCount.TabIndex = 28;
            this.toolTip1.SetToolTip(this.nudPasswordMinAlphabeticCount, "Enter the minimum alphabets allowed in the password");
            // 
            // nudPasswordMaxLength
            // 
            this.nudPasswordMaxLength.Location = new System.Drawing.Point(163, 73);
            this.nudPasswordMaxLength.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMaxLength.Name = "nudPasswordMaxLength";
            this.nudPasswordMaxLength.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMaxLength.TabIndex = 27;
            this.toolTip1.SetToolTip(this.nudPasswordMaxLength, "Enter the maximum length allowed in the password");
            // 
            // nudPasswordMaxIdenticalAdjacentChars
            // 
            this.nudPasswordMaxIdenticalAdjacentChars.Location = new System.Drawing.Point(163, 47);
            this.nudPasswordMaxIdenticalAdjacentChars.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudPasswordMaxIdenticalAdjacentChars.Name = "nudPasswordMaxIdenticalAdjacentChars";
            this.nudPasswordMaxIdenticalAdjacentChars.Size = new System.Drawing.Size(86, 20);
            this.nudPasswordMaxIdenticalAdjacentChars.TabIndex = 26;
            this.toolTip1.SetToolTip(this.nudPasswordMaxIdenticalAdjacentChars, "Enter the maximum identical adjacent characters allowed in the password");
            // 
            // txtPasswordDescription
            // 
            this.txtPasswordDescription.Location = new System.Drawing.Point(65, 21);
            this.txtPasswordDescription.Name = "txtPasswordDescription";
            this.txtPasswordDescription.Size = new System.Drawing.Size(184, 20);
            this.txtPasswordDescription.TabIndex = 25;
            this.toolTip1.SetToolTip(this.txtPasswordDescription, "Enter the description for the password policy");
            // 
            // nudTokenHokMaxRefreshLifetimeMillis
            // 
            this.nudTokenHokMaxRefreshLifetimeMillis.Location = new System.Drawing.Point(148, 238);
            this.nudTokenHokMaxRefreshLifetimeMillis.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudTokenHokMaxRefreshLifetimeMillis.Name = "nudTokenHokMaxRefreshLifetimeMillis";
            this.nudTokenHokMaxRefreshLifetimeMillis.Size = new System.Drawing.Size(86, 20);
            this.nudTokenHokMaxRefreshLifetimeMillis.TabIndex = 11;
            this.toolTip1.SetToolTip(this.nudTokenHokMaxRefreshLifetimeMillis, "Enter the maximum HOK token refresh lifetime in milli seconds");
            // 
            // nudTokenBearerMaxRefreshLifetimeMillis
            // 
            this.nudTokenBearerMaxRefreshLifetimeMillis.Location = new System.Drawing.Point(150, 156);
            this.nudTokenBearerMaxRefreshLifetimeMillis.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudTokenBearerMaxRefreshLifetimeMillis.Name = "nudTokenBearerMaxRefreshLifetimeMillis";
            this.nudTokenBearerMaxRefreshLifetimeMillis.Size = new System.Drawing.Size(86, 20);
            this.nudTokenBearerMaxRefreshLifetimeMillis.TabIndex = 9;
            this.toolTip1.SetToolTip(this.nudTokenBearerMaxRefreshLifetimeMillis, "Enter the maximum Bearer token refresh lifetime in milli seconds");
            // 
            // nudTokenRenewCount
            // 
            this.nudTokenRenewCount.Location = new System.Drawing.Point(149, 77);
            this.nudTokenRenewCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudTokenRenewCount.Name = "nudTokenRenewCount";
            this.nudTokenRenewCount.Size = new System.Drawing.Size(86, 20);
            this.nudTokenRenewCount.TabIndex = 7;
            this.toolTip1.SetToolTip(this.nudTokenRenewCount, "Enter the renew count");
            // 
            // nudTokenHokMaxLifetimeMillis
            // 
            this.nudTokenHokMaxLifetimeMillis.Location = new System.Drawing.Point(147, 212);
            this.nudTokenHokMaxLifetimeMillis.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudTokenHokMaxLifetimeMillis.Name = "nudTokenHokMaxLifetimeMillis";
            this.nudTokenHokMaxLifetimeMillis.Size = new System.Drawing.Size(86, 20);
            this.nudTokenHokMaxLifetimeMillis.TabIndex = 10;
            this.toolTip1.SetToolTip(this.nudTokenHokMaxLifetimeMillis, "Enter the maximum HOK token lifetime in milli seconds");
            // 
            // nudTokenBearerMaxLifetimeMillis
            // 
            this.nudTokenBearerMaxLifetimeMillis.Location = new System.Drawing.Point(148, 129);
            this.nudTokenBearerMaxLifetimeMillis.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudTokenBearerMaxLifetimeMillis.Name = "nudTokenBearerMaxLifetimeMillis";
            this.nudTokenBearerMaxLifetimeMillis.Size = new System.Drawing.Size(86, 20);
            this.nudTokenBearerMaxLifetimeMillis.TabIndex = 8;
            this.toolTip1.SetToolTip(this.nudTokenBearerMaxLifetimeMillis, "Enter the maximum Bearer token lifetime in milli seconds");
            // 
            // nudTokenDelegationCount
            // 
            this.nudTokenDelegationCount.Location = new System.Drawing.Point(150, 49);
            this.nudTokenDelegationCount.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.nudTokenDelegationCount.Name = "nudTokenDelegationCount";
            this.nudTokenDelegationCount.Size = new System.Drawing.Size(86, 20);
            this.nudTokenDelegationCount.TabIndex = 6;
            this.toolTip1.SetToolTip(this.nudTokenDelegationCount, "Enter the delegation count");
            // 
            // nudTokenClockTolerenceMillis
            // 
            this.nudTokenClockTolerenceMillis.Location = new System.Drawing.Point(151, 22);
            this.nudTokenClockTolerenceMillis.Maximum = new decimal(new int[] {
            -858993460,
            214748364,
            0,
            0});
            this.nudTokenClockTolerenceMillis.Name = "nudTokenClockTolerenceMillis";
            this.nudTokenClockTolerenceMillis.Size = new System.Drawing.Size(86, 20);
            this.nudTokenClockTolerenceMillis.TabIndex = 5;
            this.toolTip1.SetToolTip(this.nudTokenClockTolerenceMillis, "Enter the clock tolerance in milli seconds");
            // 
            // btnRemoveCert
            // 
            this.btnRemoveCert.Enabled = false;
            this.btnRemoveCert.Location = new System.Drawing.Point(256, 471);
            this.btnRemoveCert.Name = "btnRemoveCert";
            this.btnRemoveCert.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveCert.TabIndex = 24;
            this.btnRemoveCert.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveCert, "Removes a certificate ");
            this.btnRemoveCert.UseVisualStyleBackColor = true;
            this.btnRemoveCert.Click += new System.EventHandler(this.btnRemoveCert_Click);
            // 
            // btnAddCert
            // 
            this.btnAddCert.Location = new System.Drawing.Point(256, 412);
            this.btnAddCert.Name = "btnAddCert";
            this.btnAddCert.Size = new System.Drawing.Size(24, 23);
            this.btnAddCert.TabIndex = 23;
            this.btnAddCert.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddCert, "Adds a certificate ");
            this.btnAddCert.UseVisualStyleBackColor = true;
            this.btnAddCert.Click += new System.EventHandler(this.btnAddCert_Click);
            // 
            // btnAuthenticationRemoveOid
            // 
            this.btnAuthenticationRemoveOid.Enabled = false;
            this.btnAuthenticationRemoveOid.Location = new System.Drawing.Point(256, 352);
            this.btnAuthenticationRemoveOid.Name = "btnAuthenticationRemoveOid";
            this.btnAuthenticationRemoveOid.Size = new System.Drawing.Size(24, 23);
            this.btnAuthenticationRemoveOid.TabIndex = 22;
            this.btnAuthenticationRemoveOid.Text = "-";
            this.toolTip1.SetToolTip(this.btnAuthenticationRemoveOid, "Removes the certificate policy OID");
            this.btnAuthenticationRemoveOid.UseVisualStyleBackColor = true;
            this.btnAuthenticationRemoveOid.Click += new System.EventHandler(this.btnAuthenticationRemoveOid_Click);
            // 
            // btnAuthenticationAddOid
            // 
            this.btnAuthenticationAddOid.Location = new System.Drawing.Point(256, 280);
            this.btnAuthenticationAddOid.Name = "btnAuthenticationAddOid";
            this.btnAuthenticationAddOid.Size = new System.Drawing.Size(24, 23);
            this.btnAuthenticationAddOid.TabIndex = 21;
            this.btnAuthenticationAddOid.Text = "+";
            this.toolTip1.SetToolTip(this.btnAuthenticationAddOid, "Adds the certificate policy OID");
            this.btnAuthenticationAddOid.UseVisualStyleBackColor = true;
            this.btnAuthenticationAddOid.Click += new System.EventHandler(this.btnAuthenticationAddOid_Click);
            // 
            // txtLogonTitle
            // 
            this.txtLogonTitle.Location = new System.Drawing.Point(52, 47);
            this.txtLogonTitle.Name = "txtLogonTitle";
            this.txtLogonTitle.Size = new System.Drawing.Size(197, 20);
            this.txtLogonTitle.TabIndex = 40;
            this.toolTip1.SetToolTip(this.txtLogonTitle, "Enter the logon banner of the brand policy");
            // 
            // chkDisableBanner
            // 
            this.chkDisableBanner.AutoSize = true;
            this.chkDisableBanner.Location = new System.Drawing.Point(55, 144);
            this.chkDisableBanner.Name = "chkDisableBanner";
            this.chkDisableBanner.Size = new System.Drawing.Size(131, 17);
            this.chkDisableBanner.TabIndex = 41;
            this.chkDisableBanner.Text = "Disable Logon Banner";
            this.toolTip1.SetToolTip(this.chkDisableBanner, "Sets the Failover to CRL");
            this.chkDisableBanner.UseVisualStyleBackColor = true;
            // 
            // chkLogonBannerCheckbox
            // 
            this.chkLogonBannerCheckbox.AutoSize = true;
            this.chkLogonBannerCheckbox.Location = new System.Drawing.Point(55, 167);
            this.chkLogonBannerCheckbox.Name = "chkLogonBannerCheckbox";
            this.chkLogonBannerCheckbox.Size = new System.Drawing.Size(144, 17);
            this.chkLogonBannerCheckbox.TabIndex = 42;
            this.chkLogonBannerCheckbox.Text = "Logon Banner Checkbox";
            this.toolTip1.SetToolTip(this.chkLogonBannerCheckbox, "Sets the Failover to CRL");
            this.chkLogonBannerCheckbox.UseVisualStyleBackColor = true;
            // 
            // chkAuthPolicyPassword
            // 
            this.chkAuthPolicyPassword.AutoSize = true;
            this.chkAuthPolicyPassword.Location = new System.Drawing.Point(6, 6);
            this.chkAuthPolicyPassword.Name = "chkAuthPolicyPassword";
            this.chkAuthPolicyPassword.Size = new System.Drawing.Size(104, 17);
            this.chkAuthPolicyPassword.TabIndex = 42;
            this.chkAuthPolicyPassword.Text = "Password based";
            this.toolTip1.SetToolTip(this.chkAuthPolicyPassword, "Sets the Failover to CRL");
            this.chkAuthPolicyPassword.UseVisualStyleBackColor = true;
            // 
            // chkAuthPolicyWindows
            // 
            this.chkAuthPolicyWindows.AutoSize = true;
            this.chkAuthPolicyWindows.Location = new System.Drawing.Point(6, 29);
            this.chkAuthPolicyWindows.Name = "chkAuthPolicyWindows";
            this.chkAuthPolicyWindows.Size = new System.Drawing.Size(102, 17);
            this.chkAuthPolicyWindows.TabIndex = 43;
            this.chkAuthPolicyWindows.Text = "Windows based";
            this.toolTip1.SetToolTip(this.chkAuthPolicyWindows, "Sets the Failover to CRL");
            this.chkAuthPolicyWindows.UseVisualStyleBackColor = true;
            // 
            // chkAuthPolicyCertificate
            // 
            this.chkAuthPolicyCertificate.AutoSize = true;
            this.chkAuthPolicyCertificate.Location = new System.Drawing.Point(6, 52);
            this.chkAuthPolicyCertificate.Name = "chkAuthPolicyCertificate";
            this.chkAuthPolicyCertificate.Size = new System.Drawing.Size(105, 17);
            this.chkAuthPolicyCertificate.TabIndex = 44;
            this.chkAuthPolicyCertificate.Text = "Certificate based";
            this.toolTip1.SetToolTip(this.chkAuthPolicyCertificate, "Sets the Failover to CRL");
            this.chkAuthPolicyCertificate.UseVisualStyleBackColor = true;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(256, 441);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(24, 23);
            this.button1.TabIndex = 45;
            this.button1.Text = "v";
            this.toolTip1.SetToolTip(this.button1, "View certificate ");
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.chkLogonBannerCheckbox);
            this.groupBox1.Controls.Add(this.chkDisableBanner);
            this.groupBox1.Controls.Add(this.txtLogonTitle);
            this.groupBox1.Controls.Add(this.label31);
            this.groupBox1.Controls.Add(this.txtBrandLogonContent);
            this.groupBox1.Controls.Add(this.txtBrandName);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(566, 332);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(262, 190);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Brand Policy";
            // 
            // label31
            // 
            this.label31.AutoSize = true;
            this.label31.Location = new System.Drawing.Point(6, 50);
            this.label31.Name = "label31";
            this.label31.Size = new System.Drawing.Size(27, 13);
            this.label31.TabIndex = 39;
            this.label31.Text = "Title";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 76);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(44, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Content";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 27);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(38, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Name:";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.button1);
            this.groupBox2.Controls.Add(this.btnAuthenticationRemoveOid);
            this.groupBox2.Controls.Add(this.lstAuthenticationCertificatePolicyOids);
            this.groupBox2.Controls.Add(this.btnAuthenticationAddOid);
            this.groupBox2.Controls.Add(this.btnRemoveCert);
            this.groupBox2.Controls.Add(this.lstAuthenticationCertificates);
            this.groupBox2.Controls.Add(this.btnAddCert);
            this.groupBox2.Controls.Add(this.txtAuthenticationCrlDistributionPointOverride);
            this.groupBox2.Controls.Add(this.label30);
            this.groupBox2.Controls.Add(this.cbAuthenticationCrlDistributionPointUsage);
            this.groupBox2.Controls.Add(this.txtAuthenticationOcspUrlOverride);
            this.groupBox2.Controls.Add(this.label29);
            this.groupBox2.Controls.Add(this.cbAuthenticationFailoverToCrl);
            this.groupBox2.Controls.Add(this.cbAuthenticationOcsp);
            this.groupBox2.Controls.Add(this.cbAuthenticationRevocationCheck);
            this.groupBox2.Controls.Add(this.groupBox9);
            this.groupBox2.Controls.Add(this.panel1);
            this.groupBox2.Controls.Add(this.label28);
            this.groupBox2.Location = new System.Drawing.Point(271, 13);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(287, 509);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Authentication Policy";
            // 
            // lstAuthenticationCertificatePolicyOids
            // 
            this.lstAuthenticationCertificatePolicyOids.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2});
            this.lstAuthenticationCertificatePolicyOids.FullRowSelect = true;
            this.lstAuthenticationCertificatePolicyOids.GridLines = true;
            this.lstAuthenticationCertificatePolicyOids.Location = new System.Drawing.Point(7, 255);
            this.lstAuthenticationCertificatePolicyOids.MultiSelect = false;
            this.lstAuthenticationCertificatePolicyOids.Name = "lstAuthenticationCertificatePolicyOids";
            this.lstAuthenticationCertificatePolicyOids.Size = new System.Drawing.Size(243, 118);
            this.lstAuthenticationCertificatePolicyOids.TabIndex = 44;
            this.lstAuthenticationCertificatePolicyOids.TabStop = false;
            this.lstAuthenticationCertificatePolicyOids.UseCompatibleStateImageBehavior = false;
            this.lstAuthenticationCertificatePolicyOids.View = System.Windows.Forms.View.Details;
            this.lstAuthenticationCertificatePolicyOids.SelectedIndexChanged += new System.EventHandler(this.lstAuthenticationCertificatePolicyOids_SelectedIndexChanged);
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Certificate Policy OID";
            this.columnHeader2.Width = 235;
            // 
            // lstAuthenticationCertificates
            // 
            this.lstAuthenticationCertificates.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lstAuthenticationCertificates.FullRowSelect = true;
            this.lstAuthenticationCertificates.GridLines = true;
            this.lstAuthenticationCertificates.Location = new System.Drawing.Point(7, 386);
            this.lstAuthenticationCertificates.MultiSelect = false;
            this.lstAuthenticationCertificates.Name = "lstAuthenticationCertificates";
            this.lstAuthenticationCertificates.Size = new System.Drawing.Size(243, 109);
            this.lstAuthenticationCertificates.TabIndex = 41;
            this.lstAuthenticationCertificates.TabStop = false;
            this.lstAuthenticationCertificates.UseCompatibleStateImageBehavior = false;
            this.lstAuthenticationCertificates.View = System.Windows.Forms.View.Details;
            this.lstAuthenticationCertificates.SelectedIndexChanged += new System.EventHandler(this.lstAuthenticationCertificates_SelectedIndexChanged);
            this.lstAuthenticationCertificates.DoubleClick += new System.EventHandler(this.lstAuthenticationCertificates_DoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Certificate";
            this.columnHeader1.Width = 235;
            // 
            // label30
            // 
            this.label30.AutoSize = true;
            this.label30.Location = new System.Drawing.Point(4, 228);
            this.label30.Name = "label30";
            this.label30.Size = new System.Drawing.Size(151, 13);
            this.label30.TabIndex = 36;
            this.label30.Text = "CRL distribution point override:";
            // 
            // label29
            // 
            this.label29.AutoSize = true;
            this.label29.Location = new System.Drawing.Point(4, 202);
            this.label29.Name = "label29";
            this.label29.Size = new System.Drawing.Size(107, 13);
            this.label29.TabIndex = 4;
            this.label29.Text = "OCSP URL Override:";
            // 
            // groupBox9
            // 
            this.groupBox9.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox9.Location = new System.Drawing.Point(9, 132);
            this.groupBox9.Name = "groupBox9";
            this.groupBox9.Size = new System.Drawing.Size(268, 5);
            this.groupBox9.TabIndex = 30;
            this.groupBox9.TabStop = false;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.chkAuthPolicyCertificate);
            this.panel1.Controls.Add(this.chkAuthPolicyWindows);
            this.panel1.Controls.Add(this.chkAuthPolicyPassword);
            this.panel1.Location = new System.Drawing.Point(6, 29);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(268, 76);
            this.panel1.TabIndex = 3;
            // 
            // label28
            // 
            this.label28.AutoSize = true;
            this.label28.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label28.Location = new System.Drawing.Point(6, 118);
            this.label28.Name = "label28";
            this.label28.Size = new System.Drawing.Size(114, 13);
            this.label28.TabIndex = 31;
            this.label28.Text = "Client Certificate Policy";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.nudLockoutAutounlockIntervalSecs);
            this.groupBox3.Controls.Add(this.nudLockoutMaxFailedAttempts);
            this.groupBox3.Controls.Add(this.nudLockoutFailedAttemptIntervalSecs);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.label6);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Controls.Add(this.txtLockoutDescription);
            this.groupBox3.Controls.Add(this.label4);
            this.groupBox3.Location = new System.Drawing.Point(12, 13);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(252, 139);
            this.groupBox3.TabIndex = 6;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Lockout Policy";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(11, 112);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(137, 13);
            this.label7.TabIndex = 10;
            this.label7.Text = "Auto-Unlock interval (secs):";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(41, 85);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(105, 13);
            this.label6.TabIndex = 8;
            this.label6.Text = "Max Failed Attempts:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(4, 58);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(146, 13);
            this.label5.TabIndex = 6;
            this.label5.Text = "Failed Attempt Interval (secs):";
            // 
            // txtLockoutDescription
            // 
            this.txtLockoutDescription.Location = new System.Drawing.Point(73, 28);
            this.txtLockoutDescription.Name = "txtLockoutDescription";
            this.txtLockoutDescription.Size = new System.Drawing.Size(167, 20);
            this.txtLockoutDescription.TabIndex = 1;
            this.txtLockoutDescription.Text = "Description of the lockout policy";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(4, 31);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(63, 13);
            this.label4.TabIndex = 4;
            this.label4.Text = "Description:";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.cbProviderSelection);
            this.groupBox4.Controls.Add(this.txtProviderAlias);
            this.groupBox4.Controls.Add(this.label32);
            this.groupBox4.Controls.Add(this.txtProviderDefault);
            this.groupBox4.Controls.Add(this.label3);
            this.groupBox4.Location = new System.Drawing.Point(12, 428);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(252, 94);
            this.groupBox4.TabIndex = 7;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Provider Policy";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 23);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(44, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Default:";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.nudPasswordProhibitedPreviousPasswordCount);
            this.groupBox5.Controls.Add(this.label27);
            this.groupBox5.Controls.Add(this.nudPasswordLifetimeDays);
            this.groupBox5.Controls.Add(this.label26);
            this.groupBox5.Controls.Add(this.nudPasswordMinUppercaseCount);
            this.groupBox5.Controls.Add(this.label25);
            this.groupBox5.Controls.Add(this.nudPasswordMinSpecialCharacterCount);
            this.groupBox5.Controls.Add(this.label24);
            this.groupBox5.Controls.Add(this.nudPasswordMinNumericCount);
            this.groupBox5.Controls.Add(this.label23);
            this.groupBox5.Controls.Add(this.nudPasswordMinLowercaseCount);
            this.groupBox5.Controls.Add(this.label22);
            this.groupBox5.Controls.Add(this.nudPasswordMinLength);
            this.groupBox5.Controls.Add(this.label21);
            this.groupBox5.Controls.Add(this.nudPasswordMinAlphabeticCount);
            this.groupBox5.Controls.Add(this.label20);
            this.groupBox5.Controls.Add(this.nudPasswordMaxLength);
            this.groupBox5.Controls.Add(this.label19);
            this.groupBox5.Controls.Add(this.nudPasswordMaxIdenticalAdjacentChars);
            this.groupBox5.Controls.Add(this.txtPasswordDescription);
            this.groupBox5.Controls.Add(this.label18);
            this.groupBox5.Controls.Add(this.label17);
            this.groupBox5.Location = new System.Drawing.Point(566, 14);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(261, 312);
            this.groupBox5.TabIndex = 7;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Password Policy";
            // 
            // label27
            // 
            this.label27.AutoSize = true;
            this.label27.Location = new System.Drawing.Point(1, 288);
            this.label27.Name = "label27";
            this.label27.Size = new System.Drawing.Size(162, 13);
            this.label27.TabIndex = 32;
            this.label27.Text = "Prohibited Prev Password Count:";
            // 
            // label26
            // 
            this.label26.AutoSize = true;
            this.label26.Location = new System.Drawing.Point(36, 260);
            this.label26.Name = "label26";
            this.label26.Size = new System.Drawing.Size(126, 13);
            this.label26.TabIndex = 30;
            this.label26.Text = "Password Lifetime (days):";
            // 
            // label25
            // 
            this.label25.AutoSize = true;
            this.label25.Location = new System.Drawing.Point(50, 234);
            this.label25.Name = "label25";
            this.label25.Size = new System.Drawing.Size(113, 13);
            this.label25.TabIndex = 28;
            this.label25.Text = "Min Uppercase Count:";
            // 
            // label24
            // 
            this.label24.AutoSize = true;
            this.label24.Location = new System.Drawing.Point(41, 207);
            this.label24.Name = "label24";
            this.label24.Size = new System.Drawing.Size(121, 13);
            this.label24.TabIndex = 26;
            this.label24.Text = "Min Special Char Count:";
            // 
            // label23
            // 
            this.label23.AutoSize = true;
            this.label23.Location = new System.Drawing.Point(62, 181);
            this.label23.Name = "label23";
            this.label23.Size = new System.Drawing.Size(100, 13);
            this.label23.TabIndex = 24;
            this.label23.Text = "Min Numeric Count:";
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(49, 155);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(113, 13);
            this.label22.TabIndex = 22;
            this.label22.Text = "Min Lowercase Count:";
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(100, 129);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(63, 13);
            this.label21.TabIndex = 20;
            this.label21.Text = "Min Length:";
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Location = new System.Drawing.Point(52, 103);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(111, 13);
            this.label20.TabIndex = 18;
            this.label20.Text = "Min Alphabetic Count:";
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(96, 77);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(66, 13);
            this.label19.TabIndex = 16;
            this.label19.Text = "Max Length:";
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(17, 50);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(148, 13);
            this.label18.TabIndex = 14;
            this.label18.Text = "Max Identical Adjacent Chars:";
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Location = new System.Drawing.Point(3, 25);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(63, 13);
            this.label17.TabIndex = 14;
            this.label17.Text = "Description:";
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.groupBox8);
            this.groupBox6.Controls.Add(this.groupBox7);
            this.groupBox6.Controls.Add(this.label16);
            this.groupBox6.Controls.Add(this.label15);
            this.groupBox6.Controls.Add(this.nudTokenHokMaxRefreshLifetimeMillis);
            this.groupBox6.Controls.Add(this.label13);
            this.groupBox6.Controls.Add(this.nudTokenBearerMaxRefreshLifetimeMillis);
            this.groupBox6.Controls.Add(this.label14);
            this.groupBox6.Controls.Add(this.nudTokenRenewCount);
            this.groupBox6.Controls.Add(this.label12);
            this.groupBox6.Controls.Add(this.nudTokenHokMaxLifetimeMillis);
            this.groupBox6.Controls.Add(this.label11);
            this.groupBox6.Controls.Add(this.nudTokenBearerMaxLifetimeMillis);
            this.groupBox6.Controls.Add(this.label10);
            this.groupBox6.Controls.Add(this.nudTokenDelegationCount);
            this.groupBox6.Controls.Add(this.label9);
            this.groupBox6.Controls.Add(this.nudTokenClockTolerenceMillis);
            this.groupBox6.Controls.Add(this.label8);
            this.groupBox6.Location = new System.Drawing.Point(12, 158);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(252, 264);
            this.groupBox6.TabIndex = 7;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Token Policy";
            // 
            // groupBox8
            // 
            this.groupBox8.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox8.Location = new System.Drawing.Point(6, 99);
            this.groupBox8.Name = "groupBox8";
            this.groupBox8.Size = new System.Drawing.Size(240, 5);
            this.groupBox8.TabIndex = 20;
            this.groupBox8.TabStop = false;
            // 
            // groupBox7
            // 
            this.groupBox7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox7.Location = new System.Drawing.Point(6, 183);
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.Size = new System.Drawing.Size(240, 5);
            this.groupBox7.TabIndex = 19;
            this.groupBox7.TabStop = false;
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label16.Location = new System.Drawing.Point(7, 104);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(72, 13);
            this.label16.TabIndex = 29;
            this.label16.Text = "Bearer Token";
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label15.Location = new System.Drawing.Point(5, 189);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(64, 13);
            this.label15.TabIndex = 28;
            this.label15.Text = "HOK Token";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(3, 241);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(139, 13);
            this.label13.TabIndex = 26;
            this.label13.Text = "Max Refresh Lifetime (millis):";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(3, 159);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(139, 13);
            this.label14.TabIndex = 24;
            this.label14.Text = "Max Refresh Lifetime (millis):";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(68, 80);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(75, 13);
            this.label12.TabIndex = 22;
            this.label12.Text = "Renew Count:";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(46, 215);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(99, 13);
            this.label11.TabIndex = 20;
            this.label11.Text = "Max Lifetime (millis):";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(43, 132);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(99, 13);
            this.label10.TabIndex = 18;
            this.label10.Text = "Max Lifetime (millis):";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(54, 52);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(92, 13);
            this.label9.TabIndex = 16;
            this.label9.Text = "Delegation Count:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(11, 25);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(138, 13);
            this.label8.TabIndex = 14;
            this.label8.Text = "Clock Tolerance (milli-secs):";
            // 
            // txtProviderAlias
            // 
            this.txtProviderAlias.Location = new System.Drawing.Point(51, 46);
            this.txtProviderAlias.Name = "txtProviderAlias";
            this.txtProviderAlias.Size = new System.Drawing.Size(181, 20);
            this.txtProviderAlias.TabIndex = 38;
            this.toolTip1.SetToolTip(this.txtProviderAlias, "Enter the default provider");
            // 
            // label32
            // 
            this.label32.AutoSize = true;
            this.label32.Location = new System.Drawing.Point(8, 50);
            this.label32.Name = "label32";
            this.label32.Size = new System.Drawing.Size(32, 13);
            this.label32.TabIndex = 37;
            this.label32.Text = "Alias:";
            // 
            // cbProviderSelection
            // 
            this.cbProviderSelection.AutoSize = true;
            this.cbProviderSelection.Location = new System.Drawing.Point(51, 71);
            this.cbProviderSelection.Name = "cbProviderSelection";
            this.cbProviderSelection.Size = new System.Drawing.Size(152, 17);
            this.cbProviderSelection.TabIndex = 46;
            this.cbProviderSelection.Text = "Enable Provider selection?";
            this.toolTip1.SetToolTip(this.cbProviderSelection, "Sets the Failover to CRL");
            this.cbProviderSelection.UseVisualStyleBackColor = true;
            // 
            // TenantConfiguration
            // 
            this.AcceptButton = this.btnApply;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(840, 563);
            this.Controls.Add(this.groupBox6);
            this.Controls.Add(this.groupBox5);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnApply);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "TenantConfiguration";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "  Tenant Configuration";
            this.Load += new System.EventHandler(this.TenantConfiguration_Load);
            ((System.ComponentModel.ISupportInitialize)(this.nudLockoutAutounlockIntervalSecs)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudLockoutMaxFailedAttempts)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudLockoutFailedAttemptIntervalSecs)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordProhibitedPreviousPasswordCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordLifetimeDays)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinUppercaseCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinSpecialCharacterCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinNumericCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinLowercaseCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinLength)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMinAlphabeticCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMaxLength)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPasswordMaxIdenticalAdjacentChars)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenHokMaxRefreshLifetimeMillis)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenBearerMaxRefreshLifetimeMillis)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenRenewCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenHokMaxLifetimeMillis)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenBearerMaxLifetimeMillis)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenDelegationCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudTokenClockTolerenceMillis)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnApply;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtBrandLogonContent;
        private System.Windows.Forms.TextBox txtBrandName;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtProviderDefault;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtLockoutDescription;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown nudLockoutAutounlockIntervalSecs;
        private System.Windows.Forms.NumericUpDown nudLockoutMaxFailedAttempts;
        private System.Windows.Forms.NumericUpDown nudLockoutFailedAttemptIntervalSecs;
        private System.Windows.Forms.NumericUpDown nudTokenClockTolerenceMillis;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.NumericUpDown nudTokenDelegationCount;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.NumericUpDown nudTokenBearerMaxLifetimeMillis;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.NumericUpDown nudTokenHokMaxLifetimeMillis;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.NumericUpDown nudTokenRenewCount;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.NumericUpDown nudTokenHokMaxRefreshLifetimeMillis;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.NumericUpDown nudTokenBearerMaxRefreshLifetimeMillis;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.GroupBox groupBox8;
        private System.Windows.Forms.GroupBox groupBox7;
        private System.Windows.Forms.TextBox txtPasswordDescription;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.NumericUpDown nudPasswordMaxIdenticalAdjacentChars;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.NumericUpDown nudPasswordMaxLength;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.NumericUpDown nudPasswordMinAlphabeticCount;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.NumericUpDown nudPasswordMinLength;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.NumericUpDown nudPasswordMinLowercaseCount;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.NumericUpDown nudPasswordMinNumericCount;
        private System.Windows.Forms.Label label23;
        private System.Windows.Forms.NumericUpDown nudPasswordMinSpecialCharacterCount;
        private System.Windows.Forms.Label label24;
        private System.Windows.Forms.NumericUpDown nudPasswordMinUppercaseCount;
        private System.Windows.Forms.Label label25;
        private System.Windows.Forms.NumericUpDown nudPasswordLifetimeDays;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.NumericUpDown nudPasswordProhibitedPreviousPasswordCount;
        private System.Windows.Forms.Label label27;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.GroupBox groupBox9;
        private System.Windows.Forms.Label label28;
        private System.Windows.Forms.CheckBox cbAuthenticationRevocationCheck;
        private System.Windows.Forms.CheckBox cbAuthenticationOcsp;
        private System.Windows.Forms.CheckBox cbAuthenticationFailoverToCrl;
        private System.Windows.Forms.TextBox txtAuthenticationOcspUrlOverride;
        private System.Windows.Forms.Label label29;
        private System.Windows.Forms.CheckBox cbAuthenticationCrlDistributionPointUsage;
        private System.Windows.Forms.TextBox txtAuthenticationCrlDistributionPointOverride;
        private System.Windows.Forms.Label label30;
        private System.Windows.Forms.Button btnAuthenticationRemoveOid;
        private System.Windows.Forms.ListView lstAuthenticationCertificatePolicyOids;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Button btnAuthenticationAddOid;
        private System.Windows.Forms.Button btnRemoveCert;
        private System.Windows.Forms.ListView lstAuthenticationCertificates;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Button btnAddCert;
        private System.Windows.Forms.CheckBox chkLogonBannerCheckbox;
        private System.Windows.Forms.CheckBox chkDisableBanner;
        private System.Windows.Forms.TextBox txtLogonTitle;
        private System.Windows.Forms.Label label31;
        private System.Windows.Forms.CheckBox chkAuthPolicyCertificate;
        private System.Windows.Forms.CheckBox chkAuthPolicyWindows;
        private System.Windows.Forms.CheckBox chkAuthPolicyPassword;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox txtProviderAlias;
        private System.Windows.Forms.Label label32;
        private System.Windows.Forms.CheckBox cbProviderSelection;
    }
}