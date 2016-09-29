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
 
using Microsoft.ManagementConsole;
using System;
using System.Linq;
using System.Drawing;
using System.Windows.Forms;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.ScopeNodes;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPscHighAvailabilitySnapIn.Utils;
using VMPSCHighAvailability.Common.Helpers;
using VMIdentity.CommonUtils.Log;

namespace VMPscHighAvailabilitySnapIn.UI
{
    public partial class InfrastructureView : UserControl, IFormViewControl
    {
        InfrastructureFormView _formView;
        public InfrastructureView()
        {
            InitializeComponent();
        }

        private void InfrastructureView_Load(object sender, EventArgs e)
        {
            Dock = DockStyle.Fill;

            var image = ResourceHelper.GetToolbarImage();
            var il = new ImageList();
            il.Images.AddStrip(image);
            lstServices.SmallImageList = il;

            var node = _formView.ScopeNode as InfrastructureNode;
            lblName.Text = node.DisplayName;
            if (node != null)
            {
                var siteName = node.GetSiteName();
                lblSitename.Text = siteName;
                txtIpAddress.Text = node.InfrastructureDto.Ip;
                UpdateServices(node.InfrastructureDto);
            }
            lblLastRefreshed.Text = DateTime.Now.ToString(Constants.DateFormat);
        }

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (InfrastructureFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
        }

        private void UpdateServices(InfrastructureDto dto)
        {
            lblStatus.Text = dto.Active ? Constants.Active : Constants.InActive;
            lblStatus.ForeColor = HealthHelper.GetHealthColor(dto.Active);
            var description = CdcDcStateHelper.GetActiveServiceDesc(dto);
            toolTip1.SetToolTip(lblStatus, description);
            lstServices.Items.Clear();
            foreach (ServiceDto service in dto.Services)
            {
                var status = service.Alive ? Constants.Active : Constants.InActive;
                var message = string.Format("Last Heartbeat for server {0} service {1} is {2}, UTC: {3}", dto.Name, service.ServiceName, service.LastHeartbeat.ToString("dd-MMM-yyyy HH:mm:ss"), DateTime.UtcNow.ToString("dd-MMM-yyyy HH:mm:ss"));
                PscHighAvailabilityAppEnvironment.Instance.Logger.Log(message, LogLevel.Info);
                var hb = DateTimeConverter.ToDurationAgo(service.LastHeartbeat);
                message = string.Format("Last Heartbeat shown on UI for server {0} service {1} is {2}",dto.Name,service.ServiceName,hb);
                PscHighAvailabilityAppEnvironment.Instance.Logger.Log(message, LogLevel.Info);
                var port = service.Port == 0 ? string.Empty : service.Port.ToString();
                var values = new string[] { service.ServiceName,service.Description, port, status, hb };
                ListViewItem item = new ListViewItem(values) { ImageIndex = (int)ImageIndex.Service };
                item.BackColor = service.Alive ? Color.LightGreen : Color.Pink;
                lstServices.Items.Add(item);
            }
            lstServices.Refresh();
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            var node = _formView.ScopeNode as InfrastructureNode;
            UpdateServices(node.InfrastructureDto);
            lblLastRefreshed.Text = DateTime.Now.ToString(Constants.DateFormat);
        }

        internal void RefreshDataSource()
        {
            var node = _formView.ScopeNode as InfrastructureNode;
            UpdateServices(node.InfrastructureDto);
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }
    }
}
