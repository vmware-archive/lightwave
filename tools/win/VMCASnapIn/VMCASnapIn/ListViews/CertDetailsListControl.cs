using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMCASnapIn.Nodes;
using VMCASnapIn.Utilities;
using System.Security.Cryptography.X509Certificates;
using VMCA.Client;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.ListViews
{
    public partial class CertDetailsListControl : UserControl, IFormViewControl
    {
        CertDetailsFormView _formView = null;
        VMCAEnumContext _context = null;
        Dictionary<int, X509Certificate2> _viewCache = new Dictionary<int, X509Certificate2>();

        const int FETCH_WINDOW_SIZE = 25;
        const int INITIAL_LIST_SIZE = 25;

        public CertDetailsListControl()
        {
            this.InitializeComponent();

            this.Dock = DockStyle.Fill;
        }

        void IFormViewControl.Initialize(FormView parent)
        {
            _formView = (CertDetailsFormView)parent;

            var il = new ImageList();
            il.Images.AddStrip(MiscUtilsService.GetToolbarImage());
            lstCertDetails.SmallImageList = il;

            InitData();
        }

        void InitData()
        {
            var node = _formView.ScopeNode as VMCACertsNode;
            var dto = node.ServerDTO;

            MMCActionHelper.CheckedExec(delegate()
            {
                _context = new VMCAEnumContext(dto.VMCAClient, (VMCA.CertificateState)node.Tag);
            });
            RefreshList();
        }

        public void RefreshList()
        {
            _viewCache.Clear();
            lstCertDetails.VirtualListSize = INITIAL_LIST_SIZE;
        }

        void FillCache(int itemIndex)
        {
            if (_viewCache.ContainsKey(itemIndex))
                return;

            MMCActionHelper.CheckedExec(delegate
            {
                var list = _context.GetCertificates();
                if (list != null)
                {
                    int i = 0;
                    foreach (var dto in list)
                    {
                        _viewCache[itemIndex + i] = dto;
                        i++;
                        if (i == 40)
                            break;
                    }
                    lstCertDetails.VirtualListSize = _viewCache.Count;
                }
            });
        }

        private void lstEventDetails_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            if (_context != null)
            {
                FillCache(e.ItemIndex);

                if (_viewCache.ContainsKey(e.ItemIndex))
                {
                    var cert = _viewCache[e.ItemIndex];
                    e.Item = new ListViewItem(cert.Subject);
                    e.Item.ImageIndex = 2;
                    e.Item.SubItems.Add(cert.Issuer);
                    e.Item.SubItems.Add(cert.NotBefore.ToShortDateString());
                    e.Item.SubItems.Add(cert.NotAfter.ToShortDateString());
                    e.Item.SubItems.Add(cert.GetKeyUsage());
                    e.Item.Tag = cert;
                }
            }
        }
    }
}
