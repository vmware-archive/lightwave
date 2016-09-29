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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class HttpTransportFormView : Form, IView
    {
        private HttpTransportCollection _httpTransportCollection;
        private SortingOrder sortingOrder;

        public HttpTransportFormView()
        {
            InitializeComponent();
        }
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            var service = ScopeNodeExtensions.GetServiceGateway();
            sortingOrder = SortingOrder.Descending;
            _httpTransportCollection = service.HttpTransport.GetAll();
            _httpTransportCollection.Sort(sortingOrder);
            propGridInput.SelectedObject = _httpTransportCollection;
            propGridInput.Refresh();
            lblItemCount.Text = _httpTransportCollection.Count.ToString();
        }

        public Dto.IDataContext DataContext
        {
            get { return _httpTransportCollection; }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            sortingOrder = (sortingOrder == SortingOrder.Descending) ? SortingOrder.Ascending : SortingOrder.Descending;
            _httpTransportCollection.Sort(sortingOrder);
            propGridInput.SelectedObject = _httpTransportCollection;
            lblItemCount.Text = _httpTransportCollection.Count.ToString();
            propGridInput.Refresh();
        }
    }
}
