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
using VmIdentity.UI.Common.Utilities;
using VMDirSchemaEditorSnapIn.Nodes;
using VMDir.Common.DTO;
using VMDirSchemaEditorSnapIn.UI;
using VMDirSchemaEditorSnapIn;
using VMDirSchemaEditorSnapIn.ListViews;



namespace VMDirSchema.UI
{
    public class VMDirSchemaMainWindowController : MainWindowCommonController
    {
        private SplitViewMMCController splitViewController;
        private SchemaFederationViewController schemaFederationViewController;
        private OutlineViewDataSource outlineViewDataSource;
        private OutlineViewNavigationController navigationController;
        private VMDirSchemaServerNode serverNode;
        private string server;
        private bool isFedView = false;
        private NSImage ConnectIcon, DisconnectIcon, ClassIcon, BaseIcon, AttributesIcon, HomeIcon, ObjectEntryIcon, DetailsIcon;
        private VMDirSchemaRootScopeNode rootNode;

        public VMDirSchemaMainWindowController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public VMDirSchemaMainWindowController(NSCoder coder)
            : base(coder)
        {
        }

        public VMDirSchemaMainWindowController()
            : base("MainWindowCommon")
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseWindow();
        }

        private void LoadIcons()
        {
            ConnectIcon = NSImage.ImageNamed("connect_32x32.png");
            DisconnectIcon = NSImage.ImageNamed("disconnect_64x.png");
            HomeIcon = NSImage.ImageNamed("global.png"); 
            BaseIcon = NSImage.ImageNamed("directoryObject.png");
            ObjectEntryIcon = NSImage.ImageNamed("service");
            DetailsIcon = NSImage.ImageNamed("Record.png");
        }

        private void InitialiseWindow()
        {
            splitViewController = new SplitViewMMCController();
            splitViewController.MainOutlineView = new CustomOutlineView();
            splitViewController.MainTableView = new CustomTableView();
            navigationController = new OutlineViewNavigationController();
            schemaFederationViewController = new SchemaFederationViewController();

            SetSubView(splitViewController.View);

            LoadIcons();
            //MainToolBar.InsertItem(VMDirSchemaConstants.FEDERATION_TOOLBAR, 3);
            SetToolBarState(false);

            ServerToolBarItem.Activated += HandleConnection;
            RefreshToolBarItem.Activated += HandleRefresh;
            SearchRecordsField.Activated += StartSearch;
            // FederationViewToolBarItem.Activated += SwitchViews;
            // FederationViewToolBarItem.Label = VMDirSchemaConstants.VMDIRSCHEMA_FEDERATION_VIEW;
            SchemaViewToolBarItem.Activated += SwitchViews;
            SearchRecordsField.PlaceholderString = VMDirSchemaConstants.VMDIRSCHEMA_SEARCH;
            SearchFieldCell.CancelButtonCell.Activated += OnCloseSearch;

            NSNotificationCenter.DefaultCenter.AddObserver((NSString)VMIdentityConstants.RELOAD_OUTLINEVIEW, ReloadOutlineView);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)VMIdentityConstants.RELOAD_TABLEVIEW, ReloadTableView);
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)VMIdentityConstants.REFRESH_UI, RefreshServerData);

        }

        public void InitialiseViews()
        {
            try
            {
                if (serverNode.IsLoggedIn == true)
                {
                    SetToolBarState(true);
                    //AppDelegate appDelegate = NSApplication.SharedApplication.Delegate as AppDelegate;
                    //appDelegate.OpenConnectionMenuItem.Hidden = true;

                    //assign datasources and prepare TableView and OutlineView
                    rootNode = new VMDirSchemaRootScopeNode(serverNode);
                    outlineViewDataSource = new OutlineViewDataSource(rootNode);
                    rootNode.AddChildren();
                    splitViewController.MainOutlineView.DataSource = outlineViewDataSource;
                    splitViewController.MainOutlineView.Activated += OnOutlineViewActivated;
                    splitViewController.MainOutlineView.SelectionDidChange += OnOutlineViewActivated;
                    splitViewController.MainOutlineView.OutlineTableColumn.DataCell = new NSBrowserCell();

                    splitViewController.MainTableView.DoubleClick += OnDoubleClickedTableView;
                    splitViewController.MainOutlineView.DoubleClick += OnDoubleClickedOutlineView;

                    this.LoggedInLabel.StringValue = VMIdentityConstants.LOGGED_IN + serverNode.ServerDTO.BindDN;
                    this.Window.Title = VMDirSchemaConstants.VMDIRSCHEMA_APPNAME;

                    VMDirSchemaSnapInEnvironment.Instance.LocalData.AddServer(serverNode.ServerDTO.Server);

                    splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = VMIdentityConstants.CONNECT_TO + serverNode.DisplayName;


                    VMDirSchemaSnapInEnvironment.Instance.LocalData.AddServer(serverNode.ServerDTO.Server);

                    //Assign delegates
                    splitViewController.MainOutlineView.Delegate = new OutlineDelegate(this);
                    splitViewController.MainTableView.Delegate = new TableDelegate(this);
                }
                else
                    throw new Exception(VMIdentityConstants.SERVER_CONNECT_ERROR);
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
            FederationViewToolBarItem.Active = state;
            SchemaViewToolBarItem.Active = state;

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
                        if (item is VMDirSchemaAttributeBaseNode)
                        {
                            DisplayAttributesTableView(item);
                        }
                        else if (item is VMDirSchemaClassEntryNode)
                        {
                            DisplayClassAttributesTableView(item);
                        }
                        else if (item is VMDirSchemaClassBaseNode)
                        {
                            DisplayClassesTableView(item);
                        }
                        else
                        {
                            //Display root node children
                            NSTableColumn col = new NSTableColumn(VMIdentityConstants.NAME_FIELD);
                            col.HeaderCell.Title = VMIdentityConstants.NAME_FIELD;
                            col.DataCell = new NSBrowserCell();
                            col.Width = 160;
                            table.AddColumn(col);
                            VMDirSchemaRootScopeNode node = item as VMDirSchemaRootScopeNode;
                            splitViewController.MainTableView.DataSource = new NodesListView(node.Children, null);
                        }
                        table.ReloadData();
                    }
                });
        }

        public void DisplayAttributesTableView(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsForAttributeEntryView();
                    splitViewController.MainTableView.DataSource = new AttributesEntryListView((item as VMDirSchemaAttributeBaseNode));
                });
        }

        public void DisplayClassAttributesTableView(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsforClassAttributeEntryview();
                    //send class entry node to fetch attributes later
                    splitViewController.MainTableView.DataSource = new ClassAttributesEntryListView((item as VMDirSchemaClassEntryNode));
                });
        }

        public void DisplayClassesTableView(NSObject item)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    PopulateTableColumnsForObjectClassesView();
                    //send class entry node to fetch attributes later
                    splitViewController.MainTableView.DataSource = new ObjectClassesListView(item as VMDirSchemaClassBaseNode);
                });
        }

        public void PopulateTableColumnsForObjectClassesView()
        {
            NSTableColumn col1 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_NAME);
            col1.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_NAME;
            col1.DataCell = new NSBrowserCell();
            col1.Width = 200;
            NSTableColumn col2 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_CLASS_TYPE);
            col2.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_CLASS_TYPE;
            col2.Width = 160;
            NSTableColumn col3 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_DESC);
            col3.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_DESC;
            col3.Width = 160;
            splitViewController.MainTableView.AddColumn(col1);
            splitViewController.MainTableView.AddColumn(col2);
            splitViewController.MainTableView.AddColumn(col3);

        }

        public void PopulateTableColumnsforClassAttributeEntryview()
        {
            NSTableColumn col1 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_NAME);
            col1.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_NAME;
            col1.DataCell = new NSBrowserCell();
            col1.Width = 200;
            NSTableColumn col2 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_ATTR_SYNTAX);
            col2.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_ATTR_SYNTAX;
            col2.Width = 160;
            NSTableColumn col3 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_OPTIONAL_ATTR);
            col3.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_OPTIONAL_ATTR;
            col3.Width = 160;

            splitViewController.MainTableView.AddColumn(col1);
            splitViewController.MainTableView.AddColumn(col2);
            splitViewController.MainTableView.AddColumn(col3);
        }

        public void PopulateTableColumnsForAttributeEntryView()
        {
            NSTableColumn col1 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_NAME);
            col1.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_NAME;
            col1.DataCell = new NSBrowserCell();
            col1.Width = 200;
            NSTableColumn col2 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_ATTR_SYNTAX);
            col2.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_ATTR_SYNTAX;
            col2.Width = 160;
            NSTableColumn col3 = new NSTableColumn(VMDirSchemaConstants.VMDIRSCHEMA_DESC);
            col3.HeaderCell.Title = VMDirSchemaConstants.VMDIRSCHEMA_DESC;
            col3.Width = 160;

            splitViewController.MainTableView.AddColumn(col1);
            splitViewController.MainTableView.AddColumn(col2);
            splitViewController.MainTableView.AddColumn(col3);
        }

        public async void ConnectToServer(string server)
        {
            var serverDTO = VMDirServerDTO.CreateInstance();
            if (!string.IsNullOrWhiteSpace(server))
                serverDTO.Server = server;
            serverNode = new VMDirSchemaServerNode(serverDTO);
            ProgressWindowController pwc = new ProgressWindowController();
            IntPtr session = new IntPtr(0);
            string[] servers = VMDirSchemaSnapInEnvironment.Instance.LocalData.GetServerArray();
            LoginWindowController lwc = new LoginWindowController(servers);
            NSApplication.SharedApplication.BeginSheet(lwc.Window, this.Window, () =>
                {
                });
            nint result = NSApplication.SharedApplication.RunModalForWindow(lwc.Window);
            try
            {
                if (result == VMIdentityConstants.DIALOGOK)
                {
                    serverNode.ServerDTO.Server = lwc.Server;
                    serverNode.ServerDTO.BindDN = lwc.UserName + "@" + lwc.DomainName;
                    serverNode.ServerDTO.Password = lwc.Password;
                    NSApplication.SharedApplication.BeginSheet(pwc.Window, this.Window as NSWindow, () =>
                        {
                        });
                    session = NSApplication.SharedApplication.BeginModalSession(pwc.Window);
                    await serverNode.DoLogin();
                    InitialiseViews();

                }
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert(VMIdentityConstants.CONNECTION_NOT_SUCCESSFUL + e.Message, string.Empty);
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
                    serverNode.ServerDTO.Connection.CloseConnection();
                }
                catch (Exception ex)
                {
                    UIErrorHelper.ShowAlert("", ex.Message);
                }
                ResetMainWindow();

                NSNotificationCenter.DefaultCenter.PostNotificationName(VMIdentityConstants.RELOAD_OUTLINEVIEW, this);
                NSNotificationCenter.DefaultCenter.PostNotificationName(VMIdentityConstants.RELOAD_TABLEVIEW, this);
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
                if (isFedView)
                    this.ReplaceSubview(schemaFederationViewController.View, splitViewController.View);
                CloseConnection();
            }
        }

        public void HandleRefresh(object sender, EventArgs e)
        {
            this.RefreshServerData(null);
        }

        public void SwitchViews(object sender, EventArgs e)
        {
            schemaFederationViewController.ServerNode = serverNode;
            if (!isFedView)
            {
                isFedView = true;
                FederationViewToolBarItem.Label = VMDirSchemaConstants.VMDIRSCHEMA_SCHEMA_VIEW;
                FederationViewToolBarItem.Image = NSImage.ImageNamed(NSImageName.ActionTemplate);
                this.ReplaceSubview(splitViewController.View, schemaFederationViewController.View);
            }
            else
            {
                isFedView = false;
                FederationViewToolBarItem.Label = VMDirSchemaConstants.VMDIRSCHEMA_FEDERATION_VIEW;
                FederationViewToolBarItem.Image = NSImage.ImageNamed(NSImageName.HomeTemplate);
                this.ReplaceSubview(schemaFederationViewController.View, splitViewController.View);
            }
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
        //TODO - add double click event for table view.

        public void OnDoubleClickedTableView(object sender, EventArgs e)
        {
            
        }

        public  void OnCloseSearch(object sender, EventArgs e)
        {
            SearchFieldCell.StringValue = string.Empty;
            RefreshTableViewsBasedOnSelection(splitViewController.MainOutlineView.SelectedRow);
        }

        public  void StartSearch(object sender, EventArgs e)
        {
            int row = (int)splitViewController.MainOutlineView.SelectedRow;
            RefreshTableViewsBasedOnSelection(row);
            if (row >= 0 && !string.IsNullOrWhiteSpace(SearchRecordsField.StringValue))
            {
                NSObject item = splitViewController.MainOutlineView.ItemAtRow((int)row);
                if ((item is VMDirSchemaAttributeBaseNode))
                {
                    var view = splitViewController.MainTableView.DataSource as AttributesEntryListView;
                    view.Entries = view.Entries.FindAll(p => p.DisplayName.StartsWith(SearchRecordsField.StringValue));
                }
                else if ((item is VMDirSchemaClassBaseNode))
                {
                    var view = splitViewController.MainTableView.DataSource as ObjectClassesListView;
                    view.Entries = view.Entries.FindAll(p => p.DisplayName.StartsWith(SearchRecordsField.StringValue));
                }
                else if ((item is VMDirSchemaClassEntryNode))
                {
                    var view = splitViewController.MainTableView.DataSource as ClassAttributesEntryListView;
                    view.AttributeEntries = view.AttributeEntries.FindAll(p => p.Name.StartsWith(SearchRecordsField.StringValue));
                }
                splitViewController.MainTableView.ReloadData();
            }
        }

        public void OnDoubleClickedOutlineView(object sender, EventArgs e)
        {
            /*int row = (int)splitViewController.MainOutlineView.SelectedRow;
            NSObject item = splitViewController.MainOutlineView.ItemAtRow(row);
            if (item is VMDNSZoneEntryNode)
            {
                VMDNSZoneEntryNode zoneNode = item as VMDNSZoneEntryNode;
                zoneNode.ShowProperties();
            }*/
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
            serverNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
            rootNode.Children.Clear();
            rootNode.AddChildren();
            ReloadOutlineView(null);
            ReloadTableView(null);
        }

        //Delegate classes

        class OutlineDelegate : NSOutlineViewDelegate
        {
            VMDirSchemaMainWindowController ob;

            public OutlineDelegate(VMDirSchemaMainWindowController ob)
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
                        if (item is VMDirSchemaAttributeBaseNode || item is VMDirSchemaClassBaseNode)
                            browserCell.Image = ob.BaseIcon;
                        else if (item is VMDirSchemaClassEntryNode)
                            browserCell.Image = ob.ObjectEntryIcon;
                        else if (item is VMDirSchemaRootScopeNode)
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
            VMDirSchemaMainWindowController ob;

            public TableDelegate(VMDirSchemaMainWindowController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell(NSTableView tableView, NSObject cell,
                                                 NSTableColumn tableColumn, nint row)
            {
                try
                {
                    if (tableColumn.Identifier == VMDirSchemaConstants.VMDIRSCHEMA_NAME)
                    {
                        NSBrowserCell browserCell = cell as NSBrowserCell;
                        if (browserCell != null)
                        {
                            browserCell.Leaf = true;
                            browserCell.Image = ob.DetailsIcon;
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


