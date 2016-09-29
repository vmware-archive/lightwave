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
using System;
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewSolutionUserForm : Form, IView
    {
        private readonly ServiceGateway _service;
        private readonly Dto.ServerDto _serverDto;
        private string _tenantName;
        private bool shouldClose = true;
        public SolutionUserDto SolutionUserDto;
        public NewSolutionUserForm(ServiceGateway service, Dto.ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            InitializeComponent();
        }

        private void btnSelectCertFile_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = "Certificate Files (*.crt)|*.crt|All Files (*.*)|*.*";
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtCertFile.Text = ofd.FileName;
                }
            }
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            shouldClose = false;
            if (ValidateUser())
            {
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
                ActionHelper.Execute(delegate
                {
                    var cert = new X509Certificate2();
                    cert.Import(txtCertFile.Text);
                    var certificate = cert.ExportToPem();

                    SolutionUserDto = new SolutionUserDto
                    {
                        Name = txtUserName.Text,
                        Description = txtDescription.Text,
                        Certificate = new CertificateDto { Encoded = certificate }
                    };
                    _service.SolutionUser.Create(_serverDto, auth.Login.TenantName, SolutionUserDto, auth.Token);
                    shouldClose = true;
                }, auth);
            }
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            SolutionUserDto = null;
            shouldClose = true;
            Close();            
        }
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            base.OnClosing(e);
            e.Cancel = !shouldClose;
            shouldClose = true;
        }
        private bool ValidateUser()
        {
            if (string.IsNullOrEmpty(txtUserName.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning("Enter a valid username");
                return false;
            }
            else
            {
                try
                {
                    if (!System.IO.File.Exists(txtCertFile.Text))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid certificate path");
                        return false;
                    }
                    var cert = new X509Certificate2();
                    cert.Import(txtCertFile.Text);
                }
                catch
                {
                    MMCDlgHelper.ShowWarning("Either the certificate path is invalid or the certificate is an invalid X509Certificate");
                    return false;
                }
            }
            return true;
        }

        public IDataContext DataContext
        {
            get { return SolutionUserDto; }
        }
    }
}

