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
using VMCertStoreSnapIn.DataSource;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.Nodes;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using System.Collections.Generic;
using AppKit;
using Foundation;
using VMCertStore.Common.DTO;
using Vecs;
using System.Threading.Tasks;

namespace VMCertStoreSnapIn
{
    public partial class MainWindowController :NSWindowController
    {
        private OutlineViewDataSource outlineViewDataSource;
        private SplitViewMMCController splitViewController;
        private OutlineViewNavigationController navigationController;
        private NSImage trustedCertsIcon, secretKeysIcon, privateEntityIcon, storeIcon, storesIcon, genericIcon;

        public VMCertStoreServerNode Servernode { get; set; }

        public  string Server { get; set; }

        #region Constructors

        // Called when created from unmanaged code
        public MainWindowController(IntPtr handle)
            : base(handle)
        {
            Initialise();
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public MainWindowController(NSCoder coder)
            : base(coder)
        {
            Initialise();
        }

        // Call to load from the XIB/NIB file
        public MainWindowController()
            : base("MainWindow")
        {
        }

        // Call to load from the XIB/NIB file
        public MainWindowController(string server)
            : base("MainWindow")
        {
            this.Server = server;
            Initialise();
        }

        public void Initialise()
        {
            navigationController = new OutlineViewNavigationController();
            VMCertStoreSnapInEnvironment.Instance.mainWindow = this.Window;
        }

        #endregion

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            //load resources
            secretKeysIcon = NSImage.ImageNamed("SecretKeys16.png");
            privateEntityIcon = NSImage.ImageNamed("Privateentity_16x16.png");
            storeIcon = NSImage.ImageNamed("certificate-store.png");
            storesIcon = NSImage.ImageNamed("vecsStore_16x16.png");
            trustedCertsIcon = NSImage.ImageNamed("TrustedCerts.png");
            genericIcon = NSImage.ImageNamed("object.png");


            SetToolBarState(false);
            (NSApplication.SharedApplication.Delegate as AppDelegate).OpenConnectionMenuItem.Hidden = true;

            //Load SplitView
            splitViewController = new SplitViewMMCController();
            splitViewController.MainOutlineView = new OutlineView();
            splitViewController.MainTableView = new VMCertStoreTableView();
            this.CustomView.AddSubview(splitViewController.View);
            SearchFieldCell.CancelButtonCell.Activated += OnCloseSearch;

            //Notifications for OutlineView and Tableview to reload
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadOutlineView", ReloadOutlineView);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadTableView", ReloadTableView);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadServerData", RefreshDataFromServer);
        }

        public void InitialiseViews()
        {
            try
            {
                Servernode.IsLoggedIn = true;
                AppDelegate appDelegate = NSApplication.SharedApplication.Delegate as AppDelegate;
                appDelegate.OpenConnectionMenuItem.Hidden = true;

                //assign datasources and prepare TableView and OutlineView
                outlineViewDataSource = new OutlineViewDataSource(new VecsStoresNode(Servernode));
                splitViewController.MainOutlineView.DataSource = outlineViewDataSource;
                splitViewController.MainOutlineView.Activated += OnOutlineViewActivated;
                splitViewController.MainOutlineView.SelectionDidChange += OnOutlineViewActivated;
                splitViewController.MainOutlineView.OutlineTableColumn.DataCell = new NSBrowserCell();
                this.LoggedInLabel.StringValue = "Logged in : " + Servernode.ServerDTO.UserName;
                splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = "Connected to " + Servernode.ServerDTO.Server;

                SetToolBarState(true);

                VMCertStoreSnapInEnvironment.Instance.LocalData.AddServer(Servernode.ServerDTO.Server);

                //Assign delegates
                splitViewController.MainOutlineView.Delegate = new OutlineDelegate(this);
                splitViewController.MainTableView.Delegate = new TableDelegate(this);
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert(e.Message, "Alert");
            }
        }

        private void SetToolBarState(bool state)
        {
            if (state == false)
            {
                ConnectionToolBarItem.Image = NSImage.ImageNamed("connect_32x32.png");
                ConnectionToolBarItem.Label = "Connect";
            }
            else
            {
                ConnectionToolBarItem.Label = "Disconnect";
                ConnectionToolBarItem.Image = NSImage.ImageNamed("disconnect_64x.png");
            }
            ConnectionToolBarItem.Active = true;
            ShowServerInfoToolBarItem.Active = state;
            SearchToolBarItem.Active = state;
            RefreshToolBarItem.Active = state;
            BackForwardToolBarItem.Active = state;

        }

        public async void ConnectToServer(string server)
        {
            var serverDTO = VMCertStoreServerDTO.CreateInstance();
            Servernode = new VMCertStoreServerNode(serverDTO);
            ProgressWindowController pwc = new ProgressWindowController();
            IntPtr session = new IntPtr(0);
            string[] servers = VMCertStoreSnapInEnvironment.Instance.LocalData.GetServerArray();
            LoginWindowController lwc = new LoginWindowController(servers);
            NSApplication.SharedApplication.BeginSheet(lwc.Window, this.Window, () =>
                {
                });
            nint result = NSApplication.SharedApplication.RunModalForWindow(lwc.Window);
            try
            {
                if (result == (nint)VMIdentityConstants.DIALOGOK)
                {
                    Servernode.ServerDTO.Server = lwc.Server;
                    Servernode.ServerDTO.DomainName = lwc.DomainName;
                    Servernode.ServerDTO.UserName = lwc.UserName;
                    Servernode.ServerDTO.Password = lwc.Password;
                    NSApplication.SharedApplication.BeginSheet(pwc.Window, this.Window as NSWindow, () =>
                        {
                        });
                    session = NSApplication.SharedApplication.BeginModalSession(pwc.Window);
                    await Servernode.ServerDTO.LogintoServer(lwc.UserName, lwc.Password, lwc.DomainName);
                    if (Servernode.ServerDTO.IsLoggedIn == true)
                    {
                        InitialiseViews();
                        NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
                        Task.Run(() => Servernode.FillServerInfo());
                    }
                    else
                        UIErrorHelper.ShowAlert("Please check the login details or the user permissions.", "Unable to Login");
                }
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert("", e.Message);
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

        private void CloseConnection()
        {
            try
            {
                //Todo - replace isClosed value with VmAfdCloseServer return value. Need to add the Api in interop.
                bool isClosed = true;
                if (isClosed)
                {
                    Servernode.IsLoggedIn = false;
                    outlineViewDataSource.RootNode.Children.Clear();
                    outlineViewDataSource = null;
                    splitViewController.MainOutlineView.DataSource = null;
                    splitViewController.MainTableView.DataSource = null;
                    this.LoggedInLabel.StringValue = "";
                    splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = "";
                    RemoveTableColumns();

                    NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
                    NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
                    SetToolBarState(false);
                }
            }
            catch (Exception ex)
            {
                UIErrorHelper.ShowAlert("", ex.Message);
            }
        }

        public void SearchCertificates(String label)
        {
            nint row = splitViewController.MainOutlineView.SelectedRow;
            if (row >= (nint)0)
            {
                NSObject item = splitViewController.MainOutlineView.ItemAtRow((int)row);
                if ((item is VecsPrivateKeysNode) || (item is VecsTrustedCertsNode) || (item is VecsSecretKeysNode))
                {
                    CertificateDetailsListView certView = splitViewController.MainTableView.DataSource as CertificateDetailsListView;
                    List<CertDTO> filteredList = certView.Entries.FindAll(x => x.Alias.Contains(label));
                    splitViewController.MainTableView.DataSource = new CertificateDetailsListView(filteredList, this.Servernode.ServerDTO, certView.Store);
                }
            }
        }

        public void DisplayRightPaneSecretKeysView(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    NSTableColumn col = new NSTableColumn("Alias");
                    col.HeaderCell.Title = "Alias";
                    col.DataCell = new NSBrowserCell();
                    col.Width = 160;
                    splitViewController.MainTableView.AddColumn(col);

                    var entriesNode = item as VecsStoreEntriesNode;
                    var storeName = entriesNode.StoreName;
                    List<CertDTO> certList = null;
                    if (Servernode.StoresInfo.ContainsKey(storeName))
                        certList = Servernode.StoresInfo[storeName].SecretKeys;

                    splitViewController.MainTableView.DataSource = new SecretKeysListView(certList,Servernode.ServerDTO, storeName);
                });
        }

        public void DisplayRightPanePrivateKeysView(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsForCertView();
                    List<CertDTO> certList = null;

                    var entriesNode = item as VecsStoreEntriesNode;
                    var storeName = entriesNode.StoreName;

                    if (Servernode.StoresInfo.ContainsKey(storeName))
                        certList = Servernode.StoresInfo[storeName].PrivateKeys;

                    splitViewController.MainTableView.DataSource = new CertificateDetailsListView(certList, Servernode.ServerDTO, storeName);
                });

        }

        public void DisplayRightPaneTrustedCertsView(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsForCertView();
                    List<CertDTO> certList = null;

                    var entriesNode = item as VecsStoreEntriesNode;
                    var storeName = entriesNode.StoreName;

                    if (Servernode.StoresInfo.ContainsKey(storeName))
                        certList = Servernode.StoresInfo[storeName].Certs;

                    splitViewController.MainTableView.DataSource = new CertificateDetailsListView(certList, Servernode.ServerDTO, storeName);
                });

        }

        public void RefreshDataFromServer(NSNotification notification)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    (this.outlineViewDataSource.RootNode as VecsStoresNode).RefreshStores();
                    RefreshTableViewsBasedOnSelection(splitViewController.MainOutlineView.SelectedRow);
                    Task.Run(() => Servernode.FillServerInfo());
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
                        if (item is VecsPrivateKeysNode)
                        {
                            SearchToolBarItem.Active = true;
                            DisplayRightPanePrivateKeysView(item);
                        }
                        else if (item is VecsSecretKeysNode)
                        {
                            SearchToolBarItem.Active = false;
                            DisplayRightPaneSecretKeysView(item);
                        }
                        else if (item is VecsTrustedCertsNode)
                        {
                            SearchToolBarItem.Active = true;
                            DisplayRightPaneTrustedCertsView(item);
                        }
                        else if (item is ChildScopeNode)
                        {
                            // Display generic childscope nodes with children
                            SearchToolBarItem.Active = false;
                            NSTableColumn col = new NSTableColumn("Name");
                            col.HeaderCell.Title = "Name";
                            col.DataCell = new NSBrowserCell();
                            col.Width = 160;
                            table.AddColumn(col);
                            ChildScopeNode node = item as ChildScopeNode;
                            splitViewController.MainTableView.DataSource = new NodesListView(node.Children, Servernode.ServerDTO);

                        }
                        else if (item is ScopeNode)
                        {
                            //Display root node children
                            SearchToolBarItem.Active = false;
                            NSTableColumn col = new NSTableColumn("Name");
                            col.HeaderCell.Title = "Name";
                            col.DataCell = new NSBrowserCell();
                            col.Width = 160;
                            table.AddColumn(col);
                            ScopeNode node = item as ScopeNode;
                            splitViewController.MainTableView.DataSource = new NodesListView(node.Children, null);

                        }
                        table.ReloadData();
                    }
                });
        }

        void PopulateTableColumnsForCertView()
        {
            NSTableColumn aliasColumn = new NSTableColumn("Alias");
            aliasColumn.HeaderCell.Title = "Alias";
            aliasColumn.DataCell = new NSBrowserCell();
            aliasColumn.Width = 160;
            NSTableColumn col1 = new NSTableColumn("IssuedBy");
            col1.HeaderCell.Title = "IssuedBy";
            col1.DataCell = new NSBrowserCell();
            col1.Width = 160;
            NSTableColumn col2 = new NSTableColumn("IssuedDate");
            col2.HeaderCell.Title = "IssuedDate";
            col2.Width = 160;
            NSTableColumn col3 = new NSTableColumn("ExpirationDate");
            col3.HeaderCell.Title = "ExpirationDate";
            col3.Width = 160;
            NSTableColumn col4 = new NSTableColumn("IntendedPurposes");
            col4.HeaderCell.Title = "IntendedPurposes";
            col4.Width = 160;
            NSTableColumn col5 = new NSTableColumn("Status");
            col5.HeaderCell.Title = "Status";
            col5.Width = 160;
            splitViewController.MainTableView.AddColumn(aliasColumn);
            splitViewController.MainTableView.AddColumn(col1);
            splitViewController.MainTableView.AddColumn(col2);
            splitViewController.MainTableView.AddColumn(col3);
            splitViewController.MainTableView.AddColumn(col4);
            splitViewController.MainTableView.AddColumn(col5);
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


        // SplitView Events

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

        // ToolBar Events

        partial void HandleConnection(Foundation.NSObject sender)
        {
            if (Servernode.IsLoggedIn == false)
            {
                ConnectToServer(null);
            }
            else
            {
                CloseConnection();
            }
        }

        public void OnCloseSearch(object sender, EventArgs e)
        {
            SearchFieldCell.StringValue = String.Empty;
            RefreshTableViewsBasedOnSelection(splitViewController.MainOutlineView.SelectedRow);
        }

        partial void StartSearch(AppKit.NSSearchField sender)
        {
            RefreshTableViewsBasedOnSelection(splitViewController.MainOutlineView.SelectedRow);
            SearchCertificates(sender.StringValue);
        }

        partial void ShowServerInfo(NSObject sender)
        {
            var popover = new NSPopover();
            popover.Behavior = NSPopoverBehavior.Transient;
            popover.ContentViewController = new ServerInfoPopOverController(this.Servernode);
            popover.Show(CoreGraphics.CGRect.Empty, (NSView)sender, NSRectEdge.MinYEdge);
        }

        partial void BackForwardAction(Foundation.NSObject sender)
        {
            NSSegmentedControl control = sender as NSSegmentedControl;

            nint selectedSeg = control.SelectedSegment;

            switch (selectedSeg)
            {
                case 0:
                    GotoNextAction();
                    break;
                case 1:
                    GotoPreviousAction();
                    break;
                default:
                    break;
            }
        }

        private void GotoPreviousAction()
        {
            this.splitViewController.MainOutlineView.DeselectAll(this);
            nint row = (nint)navigationController.GetPreviousSelectedRow();
            //if (row > (nint)0) {
            this.splitViewController.MainOutlineView.SelectRow(row, true);
            // ToggleToolbarState (ForwardToolBarItem, true);
            // } else
            //  ToggleToolbarState (PreviousToolBarItem, false);
        }

        public void ToggleToolbarState(ActivatableToolBarItem toolbarItem, bool state)
        {
            toolbarItem.Active = state;
        }

        private void GotoNextAction()
        {
            this.splitViewController.MainOutlineView.DeselectAll(this);
            nint row = (nint)navigationController.GetForwardSelectedRow();
            // if (row > (nint)0) {
            this.splitViewController.MainOutlineView.SelectRow(row, true);
            //  PreviousToolBarItem.Active = true;
            // } else
            //   ForwardToolBarItem.Active = false;
        }

        partial void OnRefresh(Foundation.NSObject sender)
        {
            RefreshDataFromServer(null);
        }

        public override void WindowDidLoad()
        {
            base.WindowDidLoad();
            ConnectToServer(Server);
        }


        //strongly typed window accessor
        public new MainWindow Window
        {
            get
            {
                return (MainWindow)base.Window;
            }
        }

        //Delegate classes

        class OutlineDelegate : NSOutlineViewDelegate
        {
            MainWindowController ob;

            public OutlineDelegate(MainWindowController ob)
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

                        if (item is VecsPrivateKeysNode)
                            browserCell.Image = ob.privateEntityIcon;
                        else if (item is VecsSecretKeysNode)
                            browserCell.Image = ob.secretKeysIcon;
                        else if (item is VecsTrustedCertsNode)
                            browserCell.Image = ob.trustedCertsIcon;
                        else if (item is VecsStoreNode)
                            browserCell.Image = ob.storeIcon;
                        else
                            browserCell.Image = ob.storesIcon;
                    }
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine("Exception in casting : " + e.Message);
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
            MainWindowController ob;

            public TableDelegate(MainWindowController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell(NSTableView tableView, NSObject cell,
                                                 NSTableColumn tableColumn, nint row)
            {
                try
                {
                    if (tableColumn.Identifier == "Name" || tableColumn.Identifier == "IssuedBy" || tableColumn.Identifier == "Alias")
                    {
                        NSBrowserCell browserCell = cell as NSBrowserCell;
                        if (browserCell != null)
                        {
                            browserCell.Leaf = true;
                            browserCell.Image = ob.genericIcon;
                        }
                    }
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine("Exception in casting : " + e.Message);
                }
            }
        }
    }
}

