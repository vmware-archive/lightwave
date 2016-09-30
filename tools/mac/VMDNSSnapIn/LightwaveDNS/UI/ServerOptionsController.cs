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
using Foundation;
using AppKit;
using VMDNS.ListViews;
using VmIdentity.UI.Common.Utilities;
using VMDNS.Common;

namespace VMDNS
{
    public partial class ServerOptionsController : NSWindowController
    {
        VMDNSServerNode ServerNode;

        public ServerOptionsController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public ServerOptionsController(NSCoder coder)
            : base(coder)
        {
        }

        public ServerOptionsController(VMDNSServerNode node)
            : base("ServerOptions")
        {
            this.ServerNode = node;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            this.ForwardersTableView.DataSource = new ForwardersListView(ServerNode.Forwarders);
        }

        partial void MoveForwarderDown(Foundation.NSObject sender)
        {
            int selectedRow = (int)this.ForwardersTableView.SelectedRow;
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
                ForwardersTableView.ReloadData();
            }

        }


        partial void MoveForwarderUp(Foundation.NSObject sender)
        {
            int selectedRow = (int)this.ForwardersTableView.SelectedRow;
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
                //this.ForwardersTableView.DataSource = new ForwardersListView(ServerNode.Forwarders);
                ForwardersTableView.ReloadData();
            }
        }

        partial void AddForwarder(Foundation.NSObject sender)
        {
            if (!string.IsNullOrWhiteSpace(ForwarderIPField.StringValue))
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        //TODO - uncomment and test when https://bugzilla.eng.vmware.com/show_bug.cgi?id=1541111 is fixed
                        ServerNode.ServerDTO.DNSClient.AddForwarder(ForwarderIPField.StringValue);
                        ServerNode.FillForwarders();
                        this.ForwardersTableView.DataSource = new ForwardersListView(ServerNode.Forwarders);
                        ForwardersTableView.ReloadData();
                    });
            }
        }

        partial void DeleteForwarder(Foundation.NSObject sender)
        {
            int row = (int)ForwardersTableView.SelectedRow;
            if (row >= 0)
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        string forwarder = ServerNode.Forwarders[row];
                        ServerNode.ServerDTO.DNSClient.DeleteForwarder(forwarder);
                        ServerNode.FillForwarders();

                        this.ForwardersTableView.DataSource = new ForwardersListView(ServerNode.Forwarders);
                        ForwardersTableView.ReloadData();
                    });
            }
        }

        partial void OnClose(Foundation.NSObject sender)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        public new ServerOptions Window
        {
            get { return (ServerOptions)base.Window; }
        }
    }
}
