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
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDNSSnapIn.Nodes;

/*
 * @author Sumalatha Abhishek
 */

namespace VMDNSSnapIn.UI
{
    public partial class ServerOptions : Form
    {
        VMDNSServerNode ServerNode;

        public ServerOptions(VMDNSServerNode node)
        {
            this.ServerNode = node;
            InitializeComponent();
            ServerNode.FillForwarders();
            this.ForwardersList.DataSource = ServerNode.Forwarders;
        }

        private void button6_Click(object sender, EventArgs e)
        {

        }

        private void MoveForwarderUp(object sender, EventArgs e)
        {
            int selectedRow = (int)this.ForwardersList.SelectedIndex;
            if (selectedRow > 0)
            {
                UIErrorHelper.CheckedExec(delegate()
                {
                    //TODO - revisit and cleanup when https://bugzilla.eng.vmware.com/show_bug.cgi?id=1541111 is fixed
                    string temp = ServerNode.Forwarders[selectedRow];
                    //ServerNode.ServerDTO.DNSClient.DeleteForwarder(selectedRow);
                    ServerNode.Forwarders.RemoveAt(selectedRow);
                    //ServerNode.ServerDTO.DNSClient.InsertForwarder(temp, selectedRow - 1);
                    ServerNode.Forwarders.Insert(selectedRow - 1, temp);
                });
                //Verify if forwarders is updated
                
            }
        }

        private void MoveForwarderDown(object sender, EventArgs e)
        {
            int selectedRow = (int)this.ForwardersList.SelectedIndex;
            if (selectedRow >= 0 && selectedRow < ServerNode.Forwarders.Count - 1)
            {
                UIErrorHelper.CheckedExec(delegate()
                {
                    //TODO - revisit and cleanup when https://bugzilla.eng.vmware.com/show_bug.cgi?id=1541111 is fixed
                    string temp = ServerNode.Forwarders[selectedRow];
                    //ServerNode.ServerDTO.DNSClient.DeleteForwarder(selectedRow);
                    ServerNode.Forwarders.RemoveAt(selectedRow);
                    //ServerNode.ServerDTO.DNSClient.InsertForwarder(temp, selectedRow + 1);
                    ServerNode.Forwarders.Insert(selectedRow + 1, temp);
                });
                //this.ForwardersTableView.DataSource = new ForwardersListView(ServerNode.Forwarders);
                //Verify if forwarders is udpated
            }

        }

        private void DeleteForwarder(object sender, EventArgs e)
        {
            int row = (int)this.ForwardersList.SelectedIndex;
            if (row >= 0)
            {
                UIErrorHelper.CheckedExec(delegate()
                {
                    string forwarder = ServerNode.Forwarders[row];
                    ServerNode.ServerDTO.DNSClient.DeleteForwarder(forwarder);
                    RefreshForwarders();
                });
            }
        }

        private void RefreshForwarders()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                ServerNode.FillForwarders();
                this.ForwardersList.DataSource = ServerNode.Forwarders;
                this.ForwardersList.Refresh();
            });
        }

        private void AddForwarder(object sender, EventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(this.ForwarderIPFieldText.Text))
            {
                UIErrorHelper.CheckedExec(delegate()
                {
                    ServerNode.ServerDTO.DNSClient.AddForwarder(this.ForwarderIPFieldText.Text);
                    RefreshForwarders();
                });
            }
        }

        private void OKButton_Click(object sender, EventArgs e)
        {
            this.Close();
            this.DialogResult = DialogResult.OK;
        }
    }
}
