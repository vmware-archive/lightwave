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
using System.ComponentModel;
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Crypto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Security.Certificate;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class SolutionUserCertProperty : UserControl
    {
        private GenericPropertyPage _parent;
        private readonly SolutionUserDto _userDto;
        private readonly IPropertyDataManager _dataManager;

        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        public SolutionUserCertProperty(IPropertyDataManager dataManager)
        {
            _dataManager = dataManager;
            _userDto = dataManager.GetData() as SolutionUserDto;
            InitializeComponent();
            PropertyPageInit();
        }

        void PropertyPageInit()
        {
            _parent = new GenericPropertyPage { Control = this };
            _parent.Apply += _parent_Apply;
            _parent.Initialize += _parent_Initialize;
        }

        void _parent_Initialize(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                BindControls();
            },null);
        }

        void _parent_Apply(object sender, CancelEventArgs e)
        {
            _dataManager.Apply(_userDto);
        }

        void BindControls()
        {
            var base64Value = CertificateHelper.PemToBase64EncodedString(_userDto.Certificate.Encoded);
            var cert = CertificateHelper.GetX509Certificate2FromString(base64Value);
            var subjectDN = ShaWithRsaSigner.GetX500SubjectDN(cert);
            txtIssuedBy.Text = cert.IssuerName.Name;
            txtValidFrom.Text = cert.NotBefore.ToString("MM-dd-yyyy hh:mm:ss");
            txtValidTo.Text = cert.NotAfter.ToString("MM-dd-yyyy hh:mm:ss");
            txtDN.Text = subjectDN;
        }

        private void btnShowCertificate_Click(object sender, EventArgs e)
        {
            var base64 = CertificateHelper.PemToBase64EncodedString(_userDto.Certificate.Encoded);
            CertificateHelper.ShowX509Certificate(base64);
        }

        private void btnSelectCert_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                using (var ofd = new OpenFileDialog())
                {
                    ofd.Filter = "Certificate Files (*.crt)|*.crt|All Files (*.*)|*.*";
                    if (ofd.ShowDialog() == DialogResult.OK)
                    {
                        var cert = new X509Certificate2();
                        cert.Import(ofd.FileName);
                        _userDto.Certificate = new CertificateDto { Encoded = cert.ExportToPem() };
                        BindControls();
                        Page.Dirty = true;
                    }
                }
            }, null);
        }
    }
}
