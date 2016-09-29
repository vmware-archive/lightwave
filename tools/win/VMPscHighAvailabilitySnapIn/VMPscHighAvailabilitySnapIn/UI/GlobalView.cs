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
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.ScopeNodes;
using VMPscHighAvailabilitySnapIn.Utils;

namespace VMPscHighAvailabilitySnapIn.UI
{
    public partial class GlobalView : UserControl, IFormViewControl
    {
        GlobalFormView _formView;

        /// <summary>
        /// Type of view
        /// </summary>
        public ViewType View { get; set; }
        
        public GlobalView()
        {
            InitializeComponent();
        }

        private void GlobalView_Load(object sender, EventArgs e)
        {
            Dock = DockStyle.Fill;
            var il = new ImageList();
            var image = ResourceHelper.GetToolbarImage();
            il.Images.AddStrip(image);
            lstdcs.SmallImageList = il;
            LoadData();
        }

        public void LoadData(bool load = true)
        {
            lstdcs.Items.Clear();

            if (!load) return;
            var isServerNode = _formView.ScopeNode is ServerNode;
            if (isServerNode)
            {
                if (lstdcs.Columns.Count < 4)
                    lstdcs.Columns.Insert(0, "column1", Constants.PscTableColumnSitenameId, 150, HorizontalAlignment.Left, -1);
            }
            else
            {
                if (lstdcs.Columns.Count > 3)
                    lstdcs.Columns.RemoveAt(0);
            }
            var dataSource = GetDataSource();

            if (dataSource != null)
            {
                foreach (NodeDto dc in dataSource.OrderByDescending(x=>x.NodeType).ToList())
                {
                    var status = dc.Active
                        ? Constants.Active
                        : Constants.InActive;

                    string[] values;
                    if (isServerNode)
                    {
                        values = new string[] { dc.Sitename, dc.Name, status, dc.NodeType.ToString() };
                    }
                    else
                    {
                        values = new string[] { dc.Name, status, dc.NodeType.ToString() };
                    }
                    var imageIndex = dc.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure
                        ? (int)ImageIndex.Infrastructure
                        : (int)ImageIndex.Management;
                    ListViewItem item = new ListViewItem(values) { Tag = dc, ImageIndex =  imageIndex};
                    item.BackColor = dc.Active ? Color.LightGreen : Color.Pink;
                    lstdcs.Items.Add(item);
                }
            }
        }

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (GlobalFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
        }

        private List<NodeDto> GetDataSource()
        {
            List<NodeDto> ds = null;
            if (_formView.ScopeNode is ServerNode)
            {
                var node = _formView.ScopeNode as ServerNode;
                if (node != null)
                    ds = node.Hosts;
            }
            else if (_formView.ScopeNode is SiteNode)
            {
                var node = _formView.ScopeNode as SiteNode;
                if (node != null)
                    ds = node.Hosts;
            }
            else if (_formView.ScopeNode is InfrastructuresNode)
            {
                var node = _formView.ScopeNode as InfrastructuresNode;
                if (node != null)
                    ds = node.Hosts;
            }
            else if (_formView.ScopeNode is ManagementsNode)
            {
                var node = _formView.ScopeNode as ManagementsNode;
                if (node != null)
                    ds = node.Hosts;
            }
            return ds;
        }

        internal void RefreshDataSource()
        {
            LoadData();
        }
    }

    public enum ViewType
    {
        Global,
        Site,
        InfraNodeGroup,
        MgmtNodeGroup        
    }
}
