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
using VmIdentity.UI.Common;
using VMDNS.Common;
using VmIdentity.UI.Common.Utilities;
using VMDNS.Nodes;
using VMDNS.ListViews;
using VMDNS.Client;
using System.Collections.Generic;

/*
 * DNS Specific MainWindowController
 *
 * @author Sumalatha Abhishek
 */


namespace VMDNS.UI
{
    public class VMDNSMainWindowController : MainWindowCommonController
    {
        private SplitViewMMCController splitViewController;
        private OutlineViewDataSource outlineViewDataSource;
        private OutlineViewNavigationController navigationController;
        private VMDNSServerNode serverNode;
        private string server;
        private NSImage ZonesIcon, ForwardZoneIcon, ReverseZoneIcon, ZoneIcon, RecordIcon, HomeIcon, ConnectIcon, DisconnectIcon;
        private VMDNSRootScopeNode rootNode;
        private  NSPopover QueryRecordPopover;
        private FilterRecordsController fwc;

        public VMDNSMainWindowController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public VMDNSMainWindowController(NSCoder coder)
            : base(coder)
        {
        }

        public VMDNSMainWindowController()
            : base("MainWindowCommon")
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseDNSWindow();
        }

        private void LoadIcons()
        {
            ZoneIcon = NSImage.ImageNamed("DNSZone.png");
            ZonesIcon = NSImage.ImageNamed("DNSZones.png");
            RecordIcon = NSImage.ImageNamed("DNSRecord.png");
            HomeIcon = NSImage.ImageNamed("DNSHome.png");
            ConnectIcon = NSImage.ImageNamed("connect_32x32.png");
            DisconnectIcon = NSImage.ImageNamed("disconnect_64x.png");
            ForwardZoneIcon = NSImage.ImageNamed("ArrowRight.png");
            ReverseZoneIcon = NSImage.ImageNamed("ArrowLeft.png");
        }

        private void InitialiseDNSWindow()
        {
            splitViewController = new SplitViewMMCController();
            splitViewController.MainOutlineView = new DNSOutlineView();
            splitViewController.MainTableView = new DNSTableView();
            navigationController = new OutlineViewNavigationController();

            SetSubView(splitViewController.View);

            LoadIcons();

            SetToolBarState(false);

            ServerToolBarItem.Activated += HandleConnection;
            RefreshToolBarItem.Activated += HandleRefresh;
            SearchRecordsField.Activated += StartSearch;
            SearchRecordsField.PlaceholderString = "Search Zones";

            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadOutlineView", ReloadOutlineView);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadTableView", ReloadTableView);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"RefreshServerData", RefreshServerData);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"NSPopoverDidCloseNotification", OnQueryRecordPopoverClosed);

        }

        public override void InitialiseViews()
        {
            try
            {
                serverNode.IsLoggedIn = true;
                SetToolBarState(true);
                //AppDelegate appDelegate = NSApplication.SharedApplication.Delegate as AppDelegate;
                //appDelegate.OpenConnectionMenuItem.Hidden = true;

                //assign datasources and prepare TableView and OutlineView
                rootNode = new VMDNSRootScopeNode(serverNode);
                outlineViewDataSource = new OutlineViewDataSource(rootNode);
                rootNode.AddChildren();
                splitViewController.MainOutlineView.DataSource = outlineViewDataSource;
                splitViewController.MainOutlineView.Activated += OnOutlineViewActivated;
                splitViewController.MainOutlineView.SelectionDidChange += OnOutlineViewActivated;
                splitViewController.MainOutlineView.OutlineTableColumn.DataCell = new NSBrowserCell();

                splitViewController.MainTableView.DoubleClick += OnDoubleClickedTableView;
                splitViewController.MainOutlineView.DoubleClick += OnDoubleClickedOutlineView;

                this.LoggedInLabel.StringValue = VMIdentityConstants.LOGGED_IN + serverNode.ServerDTO.UserName;
                this.Window.Title = VMDNSConstants.VMDNS_APPNAME;

                VMDNSSnapInEnvironment.Instance.LocalData.AddServer(serverNode.ServerDTO.Server);

                splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = VMIdentityConstants.CONNECT_TO + "DNS Server";


                VMDNSSnapInEnvironment.Instance.LocalData.AddServer(serverNode.ServerDTO.Server);

                //Assign delegates
                splitViewController.MainOutlineView.Delegate = new OutlineDelegate(this);
                splitViewController.MainTableView.Delegate = new TableDelegate(this);
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert(string.Empty, e.Message);
            }
        }

        private void SetToolBarState(bool state)
        {
            ServerToolBarItem.Active = true;
            if (state == false)
            {
                ServerToolBarItem.Image = ConnectIcon;
                ServerToolBarItem.Label = VMIdentityConstants.CONNECT;
            }
            else
            {
                ServerToolBarItem.Label = VMIdentityConstants.DISCONNECT;
                ServerToolBarItem.Image = DisconnectIcon;
            }

            RefreshToolBarItem.Active = state;
            SearchToolBarItem.Active = state;
            BackForwardToolBarItem.Active = state;

        }

        void RemoveTableColumns()
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    while (splitViewController.MainTableView.ColumnCount > 0)
                    {
                        splitViewController.MainTableView.RemoveColumn(splitViewController.MainTableView.TableColumns()[0]);
                    }
                });
        }

        public void RefreshTableViewsBasedOnSelection(nint row)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    if (row >= (nint)0)
                    {
                        NSObject item = splitViewController.MainOutlineView.ItemAtRow((int)row);
                        NSTableView table = splitViewController.MainTableView;
                        RemoveTableColumns();
                        if (item is VMDNSForwardZonesNode || item is VMDNSReverseZonesNode)
                        {
                            DisplayZoneDetails(item);
                        }
                        else if (item is VMDNSZoneEntryNode)
                        {
                            DisplayRecordDetails(item);
                        }
                        else
                        {
                            //Display root node children
                            NSTableColumn col = new NSTableColumn(VMIdentityConstants.NAME_FIELD);
                            col.HeaderCell.Title = VMIdentityConstants.NAME_FIELD;
                            col.DataCell = new NSBrowserCell();
                            col.Width = 160;
                            table.AddColumn(col);
                            VMDNSRootScopeNode node = item as VMDNSRootScopeNode;
                            splitViewController.MainTableView.DataSource = new NodesListView(node.Children, null);
                        }
                        table.ReloadData();
                    }
                });
        }

        public void DisplayZoneDetails(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsForZoneView();
                    splitViewController.MainTableView.DataSource = new ZoneDetailsListView(item as VMDNSZonesBaseNode);
                });
        }

        public void DisplayRecordDetails(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsForRecordView();
                    splitViewController.MainTableView.DataSource = new DnsRecordListView((item as VMDNSZoneEntryNode));
                });
        }

        public void PopulateTableColumnsForRecordView()
        {
            NSTableColumn col1 = new NSTableColumn(VMDNSConstants.RECORD_NAME);
            col1.HeaderCell.Title = VMDNSConstants.RECORD_NAME;
            col1.DataCell = new NSBrowserCell();
            col1.Width = 200;
            NSTableColumn col2 = new NSTableColumn(VMDNSConstants.RECORD_TYPE);
            col2.HeaderCell.Title = VMDNSConstants.RECORD_TYPE;
            col2.Width = 160;
            splitViewController.MainTableView.AddColumn(col1);
            splitViewController.MainTableView.AddColumn(col2);

        }

        public void PopulateTableColumnsForZoneView()
        {
            NSTableColumn col1 = new NSTableColumn(VMDNSConstants.ZONE_NAME);
            col1.HeaderCell.Title = VMDNSConstants.ZONE_NAME;
            col1.DataCell = new NSBrowserCell();
            col1.Width = 200;
            NSTableColumn col2 = new NSTableColumn(VMDNSConstants.DNS_NAME);
            col2.HeaderCell.Title = VMDNSConstants.DNS_NAME;
            col2.Width = 160;
            NSTableColumn col3 = new NSTableColumn(VMDNSConstants.ADMIN_EMAIL);
            col3.HeaderCell.Title = VMDNSConstants.ADMIN_EMAIL;
            col3.Width = 160;
            splitViewController.MainTableView.AddColumn(col1);
            splitViewController.MainTableView.AddColumn(col2);
            splitViewController.MainTableView.AddColumn(col3);
        }

        public async void ConnectToServer(string server)
        {
            var serverDTO = VMDNSServerDTO.CreateInstance();
            serverNode = new VMDNSServerNode(serverDTO);
            string[] servers = VMDNSSnapInEnvironment.instance.LocalData.GetServerArray();
            ProgressWindowController pwc = new ProgressWindowController();
            IntPtr session = new IntPtr(0);
            LoginWindowController lwc = new LoginWindowController(servers);
            NSApplication.SharedApplication.BeginSheet(lwc.Window, this.Window, () =>
                {
                });
            nint result = NSApplication.SharedApplication.RunModalForWindow(lwc.Window);
            try
            {
                if (result == (nint)VMIdentityConstants.DIALOGOK)
                {
                    serverNode.ServerDTO.Server = lwc.Server;
                    serverNode.ServerDTO.DomainName = lwc.DomainName;
                    serverNode.ServerDTO.UserName = lwc.UserName;
                    serverNode.ServerDTO.Password = lwc.Password;
                    NSApplication.SharedApplication.BeginSheet(pwc.Window, this.Window as NSWindow, () =>
                        {
                        });
                    session = NSApplication.SharedApplication.BeginModalSession(pwc.Window);
                    //use await here and make it async
                    await serverNode.ServerDTO.LoginToServer();
                    if (serverNode.ServerDTO.IsLoggedIn == true)
                    {
                        serverNode.FillZonesInfo();
                        InitialiseViews();
                        ReloadOutlineView(null);

                    }
                    else
                        UIErrorHelper.ShowAlert(string.Empty, VMIdentityConstants.SERVER_CONNECT_ERROR);
                }
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert(string.Empty, e.Message);
            }
            finally
            {
                if (pwc.ProgressBar != null)
                {
                    pwc.ProgressBar.StopAnimation(pwc.Window);
                    pwc.Window.Close();
                    NSApplication.SharedApplication.EndModalSession(session);
                }
                Window.EndSheet(lwc.Window);
                lwc.Dispose();
            }
        }

        public void SetActiveServer(string host)
        {
            this.server = host;
        }

        private void CloseConnection()
        {
            if (serverNode.IsLoggedIn)
            {
                try
                {
                    serverNode.ServerDTO.DNSClient.CloseConnection();
                }
                catch (Exception ex)
                {
                    UIErrorHelper.ShowAlert("", ex.Message);
                }
                ResetMainWindow();

                NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
                NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
            }
        }

        private void ResetMainWindow()
        {
            serverNode.IsLoggedIn = false;
            if (outlineViewDataSource != null)
            {
                outlineViewDataSource.RootNode.Children.Clear();
                outlineViewDataSource = null;
            }
            splitViewController.MainOutlineView.DataSource = null;
            splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = string.Empty;

            splitViewController.MainTableView.DataSource = null;
            RemoveTableColumns();
            LoggedInLabel.StringValue = string.Empty;
            SetToolBarState(false);
        }

        public void HandleConnection(object sender, EventArgs e)
        {
            if (serverNode.IsLoggedIn == false)
            {
                ConnectToServer(null);
            }
            else
            {
                CloseConnection();
            }
        }

        public void HandleRefresh(object sender, EventArgs e)
        {
            this.RefreshServerData(null);
        }

        public override void WindowDidLoad()
        {
            base.WindowDidLoad();
            ConnectToServer(server);
        }

        public new MainWindowCommon Window
        {
            get { return (MainWindowCommon)base.Window; }
        }

        // SplitView Events

        public void OnDoubleClickedTableView(object sender, EventArgs e)
        {
            int row = (int)splitViewController.MainTableView.SelectedRow;
            INSTableViewDataSource ds = splitViewController.MainTableView.DataSource;
            if (ds is ZoneDetailsListView)
            {
                VMDNSZoneEntryNode zoneNode = (ds as ZoneDetailsListView).Entries[row];
                zoneNode.ShowProperties();
            }
            else if (ds is DnsRecordListView)
            {
                VMDNSZoneEntryNode zoneNode = (ds as DnsRecordListView).ZoneNode;
                VmDnsRecord record = (ds as DnsRecordListView).Entries[(int)row];
                zoneNode.ShowRecordProperties(sender, e, record);
            }
        }

        public  void StartSearch(object sender, EventArgs e)
        {
            int row = (int)splitViewController.MainOutlineView.SelectedRow;
            NSObject item = splitViewController.MainOutlineView.ItemAtRow(row);
            if (item is VMDNSZoneEntryNode)
            {
                QueryRecordPopover = new NSPopover();
                QueryRecordPopover.Behavior = NSPopoverBehavior.Transient;
                fwc = new FilterRecordsController((item as VMDNSZoneEntryNode).CurrentZone.Name, SearchRecordsField.StringValue, QueryRecordPopover);
                QueryRecordPopover.ContentViewController = fwc;
                QueryRecordPopover.Show(CoreGraphics.CGRect.Empty, (NSView)sender, NSRectEdge.MinYEdge);
            }
            else
                UIErrorHelper.ShowAlert("", VMDNSConstants.SELECT_ZONE);
        }

        public void OnQueryRecordPopoverClosed(NSNotification notification)
        {
            if (QueryRecordPopover != null && fwc != null)
            {
                if (!string.IsNullOrWhiteSpace(fwc.RecordName) && !string.IsNullOrWhiteSpace(fwc.ZoneName))
                {
                    UIErrorHelper.CheckedExec(delegate()
                        {
                            int row = (int)splitViewController.MainOutlineView.SelectedRow;
                            NSObject item = splitViewController.MainOutlineView.ItemAtRow(row);
                            if (item is VMDNSZoneEntryNode)
                            {
                                var zone = (item as VMDNSZoneEntryNode).CurrentZone;
                                if (zone != null)
                                {
                                    IList<VmDnsRecord> records =
                                        zone.QueryRecords(
                                            fwc.RecordName,
                                            fwc.RecordType,
                                            0);
                                    splitViewController.MainTableView.DataSource = new DnsRecordListView(item as VMDNSZoneEntryNode, records);
                                    splitViewController.MainTableView.ReloadData();
                                }
                            }
                        });
                }
            }
        }

        public void OnDoubleClickedOutlineView(object sender, EventArgs e)
        {
            int row = (int)splitViewController.MainOutlineView.SelectedRow;
            NSObject item = splitViewController.MainOutlineView.ItemAtRow(row);
            if (item is VMDNSZoneEntryNode)
            {
                VMDNSZoneEntryNode zoneNode = item as VMDNSZoneEntryNode;
                zoneNode.ShowProperties();
            }
        }

        public void OnOutlineViewActivated(object sender, EventArgs e)
        {
            nint row = splitViewController.MainOutlineView.SelectedRow;
            if (row > (nint)0)
                navigationController.AddPreviousSelectedRow((int)row);
        }

        public void ReloadOutlineView(NSNotification notification)
        {
            splitViewController.MainOutlineView.ReloadData();
        }

        public void ReloadTableView(NSNotification notification)
        {
            RefreshTableViewsBasedOnSelection(splitViewController.MainOutlineView.SelectedRow);
        }

        public void RefreshServerData(NSNotification notification)
        {
            rootNode.Children.Clear();
            rootNode.AddChildren();
            ReloadOutlineView(null);
        }



        //Delegate classes

        class OutlineDelegate : NSOutlineViewDelegate
        {
            VMDNSMainWindowController ob;

            public OutlineDelegate(VMDNSMainWindowController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell(NSOutlineView outlineView, NSObject cell,
                                                 NSTableColumn tableColumn, NSObject item)
            {
                try
                {
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null)
                    {
                        browserCell.Leaf = true;
                        if (item is VMDNSForwardZonesNode)
                            browserCell.Image = ob.ForwardZoneIcon;
                        else if (item is VMDNSReverseZonesNode)
                            browserCell.Image = ob.ReverseZoneIcon;
                        else if (item is VMDNSZonesBaseNode)
                            browserCell.Image = ob.ZonesIcon;
                        else if (item is VMDNSZoneEntryNode)
                            browserCell.Image = ob.ZoneIcon;
                        else
                            browserCell.Image = ob.HomeIcon;

                    }

                }
                catch (Exception e)
                {
                }
            }

            public override void SelectionDidChange(NSNotification notification)
            {
                nint row = ob.splitViewController.MainOutlineView.SelectedRow;
                ob.RefreshTableViewsBasedOnSelection(row);
            }

        }

        class TableDelegate : NSTableViewDelegate
        {
            VMDNSMainWindowController ob;

            public TableDelegate(VMDNSMainWindowController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell(NSTableView tableView, NSObject cell,
                                                 NSTableColumn tableColumn, nint row)
            {
                try
                {
                    if (tableColumn.Identifier == VMDNSConstants.RECORD_NAME)
                    {
                        NSBrowserCell browserCell = cell as NSBrowserCell;
                        if (browserCell != null)
                        {
                            browserCell.Leaf = true;
                            browserCell.Image = ob.RecordIcon;
                        }
                    }
                    else if (tableColumn.Identifier == VMDNSConstants.ZONE_NAME || tableColumn.Identifier == VMIdentityConstants.NAME_FIELD)
                    {
                        NSBrowserCell browserCell = cell as NSBrowserCell;
                        if (browserCell != null)
                        {
                            browserCell.Leaf = true;
                            browserCell.Image = ob.ZoneIcon;
                        }
                    }
                }
                catch (Exception e)
                {
                }
            }
        }
    }
}

