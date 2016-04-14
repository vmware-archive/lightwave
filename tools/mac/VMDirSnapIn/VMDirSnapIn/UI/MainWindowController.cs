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

using System;
using AppKit;
using Foundation;
using Nodes;
using VMDir.Common.DTO;
using VMDirSnapIn.DataSource;
using VMDirSnapIn.Nodes;
using VMDirSnapIn.UI;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class MainWindowController : NSWindowController
    {
        private SplitViewMMCController splitViewController;

        private OutlineViewDataSource outlineViewDataSource;

        private OutlineViewNavigationController navigationController;

        private NSTableView MainTableView;

        private NSOutlineView MainOutlineView;
        private VMDirServerInfo serverNode;

        //observers
        private NSObject ReloadOutlineViewNotificationObject;
        private NSObject ReloadTableViewNotificationObject;
        private NSObject CloseNotificationObject;


        private  string server { get; set; }


        #region Constructors

        // Called when created from unmanaged code
        public MainWindowController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public MainWindowController(NSCoder coder)
            : base(coder)
        {
        }

        // Call to load from the XIB/NIB file
        public MainWindowController()
            : base("MainWindow")
        {
            Initialise();
        }

        // Call to load from the XIB/NIB file
        public MainWindowController(string serverName)
            : base("MainWindow")
        {
            Initialise();
            server = serverName;
        }

        private void Initialise()
        {
            var serverDTO = VMDirServerDTO.CreateInstance();
            serverNode = new VMDirServerInfo(serverDTO);
            navigationController = new OutlineViewNavigationController();

        }

        #endregion

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();

            try
            {
                Window.SetContentBorderThickness(24, NSRectEdge.MinYEdge);
                VMDirSnapInEnvironment.Instance.MainWindow = this.Window;

                //Load SplitView
                splitViewController = new SplitViewMMCController();
                splitViewController.MainOutlineView = new OutlineView();
                splitViewController.MainTableView = new VMDirTableView();
                this.ContainerView.AddSubview(splitViewController.View);
                SetToolBarState(false);
                (NSApplication.SharedApplication.Delegate as AppDelegate).OpenConnectionMenuITem.Hidden = true;

                //Notifications for OutlineView and Tableview to reload
                ReloadOutlineViewNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadOutlineView", ReloadOutlineView);
                ReloadTableViewNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadTableView", ReloadTableView);
                CloseNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"CloseApplication", OnCloseConnectionNotificationReceived);

            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("Error : " + e.Message);
                UIErrorHelper.ShowAlert("", e.Message);
            }
        }

        private void InitialiseViews()
        {
            AppDelegate appDelegate = NSApplication.SharedApplication.Delegate  as AppDelegate;
            appDelegate.OpenConnectionMenuITem.Hidden = true;
            try
            {
                if (serverNode.IsLoggedIn)
                {
                    InitialiseDefaultOutlineView();
                    VMDirSnapInEnvironment.Instance.LocalData.AddServer(serverNode.DTO.Server);
                    DirectoryNode baseNode = new DirectoryNode(serverNode.DTO.BaseDN, serverNode.DTO, null);
                    baseNode.IsBaseNode = true;
                    outlineViewDataSource = new OutlineViewDataSource(baseNode);
                    splitViewController.MainOutlineView.DataSource = outlineViewDataSource;
                    baseNode.PopulateChildren(serverNode.DTO.BaseDN);
                    SetToolBarState(true);
                    InitialiseDefaultTableView();
                    StatusLabel.StringValue = "Logged in : " + serverNode.DTO.BindDN;
                }
                else
                    UIErrorHelper.ShowAlert("Please check your server details and credentials.", "Login not successful!");
            }
            catch (Exception e)
            {
                CloseConnection();
                UIErrorHelper.ShowAlert(e.Message, "Error in populating the directory. Please check the login details and try again");
            }
        }

        private void InitialiseDefaultOutlineView()
        {
            MainOutlineView = splitViewController.MainOutlineView;
            MainOutlineView.OutlineTableColumn.HeaderCell.Title = " Connected to " + serverNode.DTO.Server;

            MainOutlineView.Activated += OnOutlineViewActivated;
            MainOutlineView.DoubleClick += OnOutlineViewDoubleClicked;
            NSTableColumn col;
            col = MainOutlineView.OutlineTableColumn;
            if (col != null)
                col.DataCell = new NSBrowserCell();
            MainOutlineView.Delegate = new OutlineDelegate(this);
            //MainOutlineView.SelectRow (0, true);
        }

        private void InitialiseDefaultTableView()
        {
            MainTableView = splitViewController.MainTableView;

            RemoveTableColumns();

            //Populate appropriate columns
            NSTableColumn col = new NSTableColumn("Key");
            col.HeaderCell.Title = "Attribute";
            col.HeaderCell.Alignment = NSTextAlignment.Center;
            col.DataCell = new NSBrowserCell();
            col.MinWidth = 200;
            col.ResizingMask = NSTableColumnResizing.UserResizingMask;
            MainTableView.AddColumn(col);

            NSTableColumn col1 = new NSTableColumn("Value");
            col1.HeaderCell.Title = "Value";
            col1.ResizingMask = NSTableColumnResizing.UserResizingMask;
            col1.HeaderCell.Alignment = NSTextAlignment.Center;
            col1.MinWidth = 200;
            MainTableView.AddColumn(col1);


            MainTableView.Delegate = new GenericTableDelegate();
        }

        public async void ConnectToServer(string server)
        {
            var serverDTO = VMDirServerDTO.CreateInstance();
            if (!string.IsNullOrWhiteSpace(server))
                serverDTO.Server = server;
            ProgressWindowController pwc = new ProgressWindowController();
            IntPtr session = new IntPtr(0);
            ConnectToLdapWindowController awc = new ConnectToLdapWindowController(serverDTO);
            NSApplication.SharedApplication.BeginSheet(awc.Window, this.Window, () =>
                {
                });
            nint result = NSApplication.SharedApplication.RunModalForWindow(awc.Window);
            try
            {
                if (result == VMIdentityConstants.DIALOGOK)
                {
                    NSApplication.SharedApplication.BeginSheet(pwc.Window, this.Window as NSWindow, () =>
                        {
                        });
                    session = NSApplication.SharedApplication.BeginModalSession(pwc.Window);
                    serverNode = new VMDirServerInfo(serverDTO);
                    await serverNode.DoLogin();
                    InitialiseViews();

                }
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert("Connection is not successful. " + e.Message, "Alert");
            }
            finally
            {
                if (pwc.ProgressBar != null)
                {
                    pwc.ProgressBar.StopAnimation(pwc.Window);
                    pwc.Window.Close();
                    NSApplication.SharedApplication.EndModalSession(session);
                }
                Window.EndSheet(awc.Window);
                awc.Dispose();
            }
        }

        private void SetToolBarState(bool state)
        {
            if (state == false)
            {
                ServerToolBarItem.Label = "Connect";
            }
            else
            {
                ServerToolBarItem.Label = "Disconnect";
            }
            ServerToolBarItem.Active = true;
            AddObjectToolBarItem.Active = state;
            PropertiesToolBarItem.Active = state;
            SchemaToolBarItem.Active = state;
            DeleteObjectToolBarItem.Active = state;
            AddUserToolBarItem.Active = state;
            AddGroupToolBarItem.Active = state;
            BackForwardToolBarItem.Active = state;
            RefreshToolBarItem.Active = state;
            SuperLogToolBarItem.Active = state;
        }

        public void OnOutlineViewDoubleClicked(object sender, EventArgs e)
        {
            NSOutlineView obj = sender as NSOutlineView;
            if (obj != null)
            {
                nint row = obj.SelectedRow;
                if (row >= 0)
                {
                    NSObject item = obj.ItemAtRow(row);

                    if (item is DirectoryNode)
                    {
                        DirectoryNode node = item as DirectoryNode;

                        LdapPropertiesWindowController awc = new LdapPropertiesWindowController(node.Name, node.ServerDTO);
                        NSApplication.SharedApplication.RunModalForWindow(awc.Window);
                        node.RefreshProperties();
                        MainTableView.DataSource = new PropertiesTableViewDataSource(node.NodeProperties);
                        RefreshTableViewBasedOnSelection(row);
                    }
                }
            }
        }

        partial void ShowSuperLogWindow(NSObject sender)
        {
            SuperLoggingBrowserWindowController awc = new SuperLoggingBrowserWindowController(serverNode.DTO);
            NSApplication.SharedApplication.BeginSheet(awc.Window, this.Window, () =>
                {
                });
            try
            {
                NSApplication.SharedApplication.RunModalForWindow(awc.Window);
            }
            finally
            {
                Window.EndSheet(awc.Window);
                awc.Dispose();
            }
        }

        partial void ShowSchema(NSObject sender)
        {
            SchemaBrowserWindowController awc = new SchemaBrowserWindowController(serverNode.DTO);
            NSApplication.SharedApplication.BeginSheet(awc.Window, this.Window, () =>
                {
                });
            try
            {
                NSApplication.SharedApplication.RunModalForWindow(awc.Window);
            }
            finally
            {
                Window.EndSheet(awc.Window);
                awc.Dispose();
            }
        }

        partial void HandleConnection(NSObject sender)
        {
            if (serverNode == null || serverNode.IsLoggedIn == false)
            {
                ConnectToServer(null);
            }
            else
            {
                ConfirmationDialogController cwc = new ConfirmationDialogController("Are you sure?");
                nint result = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
                if (result == (nint)VMIdentityConstants.DIALOGOK)
                {
                    CloseConnection();
                }
            }
        }

        partial void StartSearch(NSSearchField sender)
        {
            nint row = MainOutlineView.SelectedRow;
            if (string.IsNullOrWhiteSpace(sender.StringValue))
            {
                //Retain selection and current datasource of outlineview
                MainOutlineView.SelectRow(row, true);
                MainTableView.ReloadData();
            }
            else
            {
                if (row >= (nint)0)
                {
                    NSObject item = MainOutlineView.ItemAtRow((int)row);
                    if ((item is DirectoryNode))
                    {
                        PropertiesTableViewDataSource certView = MainTableView.DataSource as PropertiesTableViewDataSource;
                        certView.data = certView.data.FindAll(p => p.Key.StartsWith(sender.StringValue));
                        MainTableView.ReloadData();

                    }
                }
            }
        }

        partial void AddObject(NSObject sender)
        {
            nint row = MainOutlineView.SelectedRow;
            if (row < (nint)0)
            {
                UIErrorHelper.ShowAlert("Please select a valid object from the tree view", "Alert");
            }
            else
            {
                DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
                node.ShowAddWindow();
            }
        }

        partial void AddUser(NSObject sender)
        {
            nint row = MainOutlineView.SelectedRow;
            if (row < (nint)0)
            {
                UIErrorHelper.ShowAlert("Please select a valid object from the tree view", "Alert");
            }
            else
            {
                DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
                node.ShowAddUser();
            }
        }

        partial void AddGroup(NSObject sender)
        {
            nint row = MainOutlineView.SelectedRow;
            if (row < (nint)0)
            {
                UIErrorHelper.ShowAlert("Please select a valid object from the tree view", "Alert");
            }
            else
            {
                DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
                node.ShowAddGroup();
            }
        }

        partial void DeleteObject(NSObject sender)
        {
            nint row = MainOutlineView.SelectedRow;
            if (row < (nint)0)
            {
                UIErrorHelper.ShowAlert("Please select a valid object from the tree view", "Alert");
            }
            else
            {
                DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
                node.PerformDelete();
            }
        }

        public void CloseConnection()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    serverNode.DTO.Connection.CloseConnection();
                    serverNode.IsLoggedIn = false;
                    ResetViews();
                });
        }

        private void ResetViews()
        {
            if (MainOutlineView != null)
            {
                MainOutlineView.DataSource = null;
                if (outlineViewDataSource.RootNode.Children != null)
                    outlineViewDataSource.RootNode.Children.Clear();
                outlineViewDataSource = null;

                MainOutlineView.OutlineTableColumn.HeaderCell.Title = string.Empty;

            }
            if (MainTableView != null)
            {
                RemoveTableColumns();
                MainTableView.DataSource = null;
            }
            Window.Title = "Lightwave Directory";
            StatusLabel.StringValue = "Logged in : none";

            NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
            NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
            SetToolBarState(false);
        }

        partial void ViewProperties(NSObject sender)
        {
            nint row = MainOutlineView.SelectedRow;
            if (row < (nint)0)
            {
                UIErrorHelper.ShowAlert("Please select a valid object from the tree view", "Alert");
            }
            else
            {
                DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
                node.ShowPropertiesWindow();
            }
        }

        private void RefreshTableViewBasedOnSelection(nint row)
        {
            if (row >= (nint)0)
            {
                NSObject item = MainOutlineView.ItemAtRow(row);
                if (item is DirectoryNode)
                {
                    DirectoryNode node = item as DirectoryNode;
                    if (node.NumberOfChildren() > 0)
                    {
                        MainTableView.DataSource = new NodesListView(node.Children);
                    }
                    else
                    {
                        MainTableView.DataSource = new PropertiesTableViewDataSource(node.NodeProperties);
                    }
                }
            }
            else
            {
                MainTableView.DataSource = null;
            }
            MainTableView.ReloadData();
        }

        //Handle the Right Panel Display logic here
        private void OnOutlineViewActivated(object sender, EventArgs e)
        {
            NSOutlineView obj = sender as NSOutlineView;
            if (obj != null)
            {
                nint row = obj.SelectedRow; 
                navigationController.AddPreviousSelectedRow((int)row);
            }
        }

        private void RemoveTableColumns()
        {
            while (MainTableView.ColumnCount > 0)
            {
                MainTableView.RemoveColumn(MainTableView.TableColumns()[0]);
            }
        }

        public void ReloadOutlineView(NSNotification notification)
        {
            MainOutlineView.ReloadData();
        }

        public void ReloadTableView(NSNotification notification)
        {
            RefreshTableViewBasedOnSelection(MainOutlineView.SelectedRow);
        }

        partial void BackForwardAction(Foundation.NSObject sender)
        {
            NSSegmentedControl control = sender as NSSegmentedControl;

            nint selectedSeg = control.SelectedSegment;

            switch (selectedSeg)
            {
                case 0:
                    GotoPreviousAction();
                    break;
                case 1:
                    GotoNextAction();
                    break;
                default:
                    break;
            }
        }

        private void GotoNextAction()
        {
            MainOutlineView.DeselectAll(this);
            nint row = (nint)navigationController.GetForwardSelectedRow();
            MainOutlineView.SelectRow(row, true);
        }

        private void GotoPreviousAction()
        {
            MainOutlineView.DeselectAll(this);
            nint row = (nint)navigationController.GetPreviousSelectedRow();
            MainOutlineView.SelectRow(row, true);
        }

        public void OnCloseConnectionNotificationReceived(NSNotification notification)
        {
            NSNotificationCenter.DefaultCenter.RemoveObserver(ReloadOutlineViewNotificationObject);
            NSNotificationCenter.DefaultCenter.RemoveObserver(ReloadTableViewNotificationObject);
            NSNotificationCenter.DefaultCenter.RemoveObserver(CloseNotificationObject);
        }

        partial void OnRefresh(Foundation.NSObject sender)
        {
            (this.outlineViewDataSource.RootNode as DirectoryNode).ReloadChildren();
        }

        //strongly typed window accessor
        public new MainWindow Window
        {
            get
            {
                return (MainWindow)base.Window;
            }
        }

        public override void WindowDidLoad()
        {
            base.WindowDidLoad();
            ConnectToServer(server);
        }

        //Delegate classes for the outlineview, tableview

        public class OutlineDelegate : NSOutlineViewDelegate
        {
            private NSImage directoryIcon, worldIcon;
            MainWindowController ob;

            public OutlineDelegate(MainWindowController ob)
            {
                this.ob = ob;
                directoryIcon = NSImage.ImageNamed("directoryObject.png");
                worldIcon = NSImage.ImageNamed("home.png");
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
                        if ((item as DirectoryNode).IsBaseNode)
                            browserCell.Image = worldIcon;
                        else
                            browserCell.Image = directoryIcon;
                    }
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine("Exception in casting : " + e.Message);
                }
            }

            public override void SelectionDidChange(NSNotification notification)
            {
                nint row = ob.MainOutlineView.SelectedRow;
                ob.RefreshTableViewBasedOnSelection(row);
            }
        }

        public class GenericTableDelegate : NSTableViewDelegate
        {
            private NSImage directoryIcon;

            public GenericTableDelegate()
            {
                directoryIcon = NSImage.ImageNamed("object.png");
            }

            public override void WillDisplayCell(NSTableView tableView, NSObject cell,
                                                 NSTableColumn tableColumn, nint row)
            {
                try
                {
                    if (tableColumn.Identifier == "Name" || tableColumn.Identifier == "Key")
                    {
                        NSBrowserCell browserCell = cell as NSBrowserCell;
                        if (browserCell != null)
                        {
                            browserCell.Leaf = true;
                            browserCell.Image = directoryIcon;
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
