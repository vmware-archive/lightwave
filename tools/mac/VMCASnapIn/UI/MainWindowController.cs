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
using System.Security.Cryptography.X509Certificates;
using AppKit;
using Foundation;
using VMCA;
using VMCASnapIn.DataSource;
using VMCASnapIn.DTO;
using VMCASnapIn.ListViews;
using VMCASnapIn.Nodes;
using VMCASnapIn.Services;
using VmIdentity.CommonUtils.Utilities;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;

namespace VMCASnapIn.UI
{
    public partial class MainWindowController : AppKit.NSWindowController
    {
        private OutlineViewDataSource outlineViewDataSource;
        private SplitViewMMCController splitViewController;
        private OutlineViewNavigationController navigationController;

        private NSImage genericIcon, caIcon, expiredCertIcon, revokedCertIcon, ActiveCertIcon, CertIcon, KeyPairIcon;

        public VMCAServerNode Servernode;

        public  string Server { get; set; }

        #region Constructors

        // Called when created from unmanaged code
        public MainWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public MainWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public MainWindowController () : base ("MainWindow")
        {
        }

        // Call to load from the XIB/NIB file
        public MainWindowController (string server) : base ("MainWindow")
        {
            Server = server;
            navigationController = new OutlineViewNavigationController ();
            VMCAAppEnvironment.Instance.MainWindow = this.Window;

        }

        public void Initialise (string server)
        {
            this.Server = server;
            //check if server is present in persistence and load the DTO
            VMCAServerDTO dto = VMCAAppEnvironment.Instance.LocalData.GetServerDTO (Server);
            if (dto == null)
                dto = VMCAServerDTO.CreateInstance ();
            Servernode = new VMCAServerNode (dto);
           
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            //load resources
            genericIcon = NSImage.ImageNamed ("object.png");
            caIcon = NSImage.ImageNamed ("certificate-authority.png");
            expiredCertIcon = NSImage.ImageNamed ("Certificate_silver.png");
            revokedCertIcon = NSImage.ImageNamed ("certificate-revoke.png");
            ActiveCertIcon = NSImage.ImageNamed ("Certificate_gold.png");
            CertIcon = NSImage.ImageNamed ("certificate.png");
            KeyPairIcon = NSImage.ImageNamed ("KeyPair.png");


            Window.SetContentBorderThickness (24, NSRectEdge.MinYEdge);
            SetToolBarState (false);

            //Load SplitView
            splitViewController = new SplitViewMMCController ();
            splitViewController.MainOutlineView = new OutlineView ();
            splitViewController.MainTableView = new VMCATableView ();
            this.ContainerView.AddSubview (splitViewController.View);

            //Notifications for OutlineView and Tableview to reload
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadOutlineView", ReloadOutlineView);
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadTableView", ReloadTableView);
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadAll", ReloadAll);
        }

        #endregion

        public async void ConnectToServer (string server)
        {
            ProgressWindowController pwc = new ProgressWindowController ();
            IntPtr session = new IntPtr (0);
            string[] servers = VMCAAppEnvironment.Instance.LocalData.GetServerArray ();
            LoginWindowController lwc = new LoginWindowController (servers);
            NSApplication.SharedApplication.BeginSheet (lwc.Window, this.Window, () => {
            });
            nint result = NSApplication.SharedApplication.RunModalForWindow (lwc.Window);
            try {
                if (result == (nint)Constants.DIALOGOK) {
                    this.Initialise (lwc.Server);
                    Servernode.ServerDTO.Server = lwc.Server;
                    Servernode.ServerDTO.DomainName = lwc.DomainName;
                    Servernode.ServerDTO.UserName = lwc.UserName;
                    Servernode.ServerDTO.Password = lwc.Password;
                    NSApplication.SharedApplication.BeginSheet (pwc.Window, this.Window as NSWindow, () => {
                    });
                    session = NSApplication.SharedApplication.BeginModalSession (pwc.Window);
                    await Servernode.ServerDTO.LogintoServer (lwc.UserName, lwc.Password, lwc.DomainName);
                    if (Servernode.ServerDTO.IsLoggedIn == true) {
                        Servernode.Initialise ();
                        InitialiseViews ();
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                    } else {
                        UIErrorHelper.ShowAlert ("", "Unable to login! Please validate the connection details.");
                    }
                }
            } catch (Exception e) {
                UIErrorHelper.ShowAlert ("", e.Message);
            } finally {
                if (pwc.ProgressBar != null) {
                    pwc.ProgressBar.StopAnimation (pwc.Window);
                    pwc.Window.Close ();
                    NSApplication.SharedApplication.EndModalSession (session);
                }
                Window.EndSheet (lwc.Window);
                lwc.Dispose ();
            }
        }

        public void InitialiseViews ()
        {
            outlineViewDataSource = new OutlineViewDataSource (Servernode);
            splitViewController.MainOutlineView.DataSource = outlineViewDataSource;
            splitViewController.MainOutlineView.Activated += OnOutlineViewActivated;
            splitViewController.MainOutlineView.SelectionDidChange += OnOutlineViewActivated;
            splitViewController.MainOutlineView.OutlineTableColumn.DataCell = new NSBrowserCell ();
            splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = " Connected to " + Servernode.ServerDTO.Server;

            SetToolBarState (true);
            (NSApplication.SharedApplication.Delegate as AppDelegate).OpenConnectionMenuItem.Hidden = true;
            LoggedUserLabel.StringValue = "Logged in : " + Servernode.ServerDTO.UserName + "@" + Servernode.ServerDTO.DomainName;

            VMCAAppEnvironment.Instance.LocalData.AddServer (Servernode.ServerDTO);
            //Assign delegates
            splitViewController.MainOutlineView.Delegate = new OutlineDelegate (this);
            splitViewController.MainTableView.Delegate = new TableDelegate (this);
            splitViewController.MainOutlineView.SelectRow (0, true);
        }

        private void SetToolBarState (bool state)
        {
            if (state == false) {
                HandleConnectionToolBarItem.Label = "Connect";
            } else {
                HandleConnectionToolBarItem.Label = "Disconnect";
            }
            HandleConnectionToolBarItem.Active = true;
            ServerInfoToolBarItem.Active = state;
            SearchToolBarItem.Active = state;
            BackForwardToolBarItem.Active = state;
            RefreshToolBarItem.Active = state;
        }

        public override void WindowDidLoad ()
        {
            base.WindowDidLoad ();
            ConnectToServer (Server);
        }

        private void CloseConnection ()
        {
            try {
                bool isClosed = true; 
                if (isClosed) {
                    Servernode.ClearServerNode ();
                    VMCAAppEnvironment.Instance.SaveLocalData ();
                    outlineViewDataSource.ClearNodes ();
                    splitViewController.MainOutlineView.DataSource = null;
                    splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = string.Empty;
                    splitViewController.MainTableView.DataSource = null;
                    RemoveTableColumns ();

                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                    SetToolBarState (false);
                    LoggedUserLabel.StringValue = string.Empty;
                }
            } catch (Exception ex) {
                UIErrorHelper.ShowAlert ("", ex.Message);
            }
        }

        //SplitView Events

        public void OnOutlineViewActivated (object sender, EventArgs e)
        {
            nint row = splitViewController.MainOutlineView.SelectedRow;
            if (row > (nint)0)
                navigationController.AddPreviousSelectedRow ((int)row);
        }

        public void ReloadOutlineView (NSNotification notification)
        {
            splitViewController.MainOutlineView.ReloadData ();
        }

        public void ReloadTableView (NSNotification notification)
        {
            splitViewController.MainTableView.ReloadData ();
        }

        partial void OnRefresh (Foundation.NSObject sender)
        {
            ReloadAll (null);
        }

        public void ReloadAll (NSNotification notification)
        {
            Servernode.Initialise ();
            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
        }

        public void DisplayRightPaneCertView (NSObject item)
        {
            try {
                PopulateTableColumnsForCertView ();
                ChildScopeNode node = item as ChildScopeNode;
                var state = (CertificateState)node.Tag;
                if ((int)state == -1) {
                    List<PrivateCertificateDTO> privateCerts = Servernode.ServerDTO.PrivateCertificates;
                    splitViewController.MainTableView.DataSource = new PrivateCertsListView (privateCerts, (item as ChildScopeNode).ServerDTO, (int)state);
                } else {
                    List<X509Certificate2> certList = null;

                    switch ((int)state) {
                    case 0:
                        certList = Servernode.ActiveCertsList;
                        break;
                    case 1:
                        certList = Servernode.RevokedCertsList;
                        break;
                    case 2:
                        certList = Servernode.ExpiredCertsList;
                        break;
                    default:
                        break;
                    }
                    splitViewController.MainTableView.DataSource = new CertificateDetailsListView (certList, (item as ChildScopeNode).ServerDTO, (int)state);
            
                }
            } catch (Exception e) {
                UIErrorHelper.ShowAlert ("", e.Message);
            }
        }

        public void DisplayRightPaneKeyPairView (NSObject item)
        {
            try {
                //add respective columns
                NSTableColumn col1 = new NSTableColumn ("CreatedDate");
                col1.HeaderCell.Title = "Created Date";
                col1.Width = 150;
                col1.DataCell = new NSBrowserCell ();
                splitViewController.MainTableView.AddColumn (col1);
                NSTableColumn col2 = new NSTableColumn ("KeyLength");
                col2.HeaderCell.Title = "Key Length";
                col2.Width = 150;
                splitViewController.MainTableView.AddColumn (col2);

                VMCAKeyPairNode node = item as VMCAKeyPairNode;
                splitViewController.MainTableView.DataSource = new KeyPairDetailListView (node.ServerDTO.KeyPairs);
            } catch (Exception e) {
                UIErrorHelper.ShowAlert ("", e.Message);
            }
        }

        public void DisplayRightPaneCSRView (NSObject item)
        {
            try {
                NSTableColumn col1 = new NSTableColumn ("CreatedDate");
                col1.HeaderCell.Title = "Created Date";
                col1.Width = 150;
                col1.DataCell = new NSBrowserCell ();
                splitViewController.MainTableView.AddColumn (col1);
                NSTableColumn col2 = new NSTableColumn ("Data");
                col2.HeaderCell.Title = "Data";
                col2.Width = 150;
                splitViewController.MainTableView.AddColumn (col2);

                VMCACSRNode node = item as VMCACSRNode;
                splitViewController.MainTableView.DataSource = new CSRDetailListView (node.ServerDTO.SigningRequests, (item as ChildScopeNode).ServerDTO);
            } catch (Exception e) {
                UIErrorHelper.ShowAlert ("", e.Message);
            }
        }

        void PopulateTableColumnsForCertView ()
        {
            NSTableColumn col1 = new NSTableColumn ("IssuedTo");
            col1.HeaderCell.Title = "IssuedTo";
            col1.Width = 150;
            col1.DataCell = new NSBrowserCell ();
            NSTableColumn col2 = new NSTableColumn ("IssuedBy");
            col2.HeaderCell.Title = "IssuedBy";
            NSTableColumn col3 = new NSTableColumn ("IssuedDate");
            col3.HeaderCell.Title = "IssuedDate";
            NSTableColumn col4 = new NSTableColumn ("ExpirationDate");
            col4.HeaderCell.Title = "ExpirationDate";
            NSTableColumn col5 = new NSTableColumn ("IntendedPurposes");
            col5.HeaderCell.Title = "IntendedPurposes";
            NSTableColumn col6 = new NSTableColumn ("Status");
            col6.HeaderCell.Title = "Status";
            splitViewController.MainTableView.AddColumn (col1);
            splitViewController.MainTableView.AddColumn (col2);
            splitViewController.MainTableView.AddColumn (col3);
            splitViewController.MainTableView.AddColumn (col4);
            splitViewController.MainTableView.AddColumn (col5);
            splitViewController.MainTableView.AddColumn (col6);
        }


        void RemoveTableColumns ()
        {
            while (splitViewController.MainTableView.ColumnCount > 0) {
                splitViewController.MainTableView.RemoveColumn (splitViewController.MainTableView.TableColumns () [0]);
            }
        }

        //ToolBar Events

        partial void ShowServerInfo (NSObject sender)
        {
            var popover = new NSPopover ();
            popover.Behavior = NSPopoverBehavior.Transient;
            popover.ContentViewController = new ServerInfoPopOverController (this.Servernode, popover);
            popover.Show (CoreGraphics.CGRect.Empty, (NSView)sender, NSRectEdge.MinYEdge);
        }

        private void GotoNextAction ()
        {
            this.splitViewController.MainOutlineView.DeselectAll (this);
            nint row = (nint)navigationController.GetForwardSelectedRow ();
            //if (row > (nint)0) {
            this.splitViewController.MainOutlineView.SelectRow (row, true);
            //BackForwardToolBarItem..Active = true;
            // } else
            //ForwardActionToolBarItem.Active = false;
        }

        private void GotoPreviousAction ()
        {
            this.splitViewController.MainOutlineView.DeselectAll (this);
            nint row = (nint)navigationController.GetPreviousSelectedRow ();
            //if (row > (nint)0) {
            this.splitViewController.MainOutlineView.SelectRow (row, true);
            //ToggleToolbarState (ForwardActionToolBarItem, true);
            //} else
            //ToggleToolbarState (PreviousActionToolBarItem, false);
        }

        partial void BackForwardAction (Foundation.NSObject sender)
        {
            NSSegmentedControl control = sender as NSSegmentedControl;

            nint selectedSeg = control.SelectedSegment;

            switch (selectedSeg) {
            case 0:
                GotoNextAction ();
                break;
            case 1:
                GotoPreviousAction ();
                break;
            default:
                break;
            }
        }

        partial void HandleConnection (Foundation.NSObject sender)
        {
            if (Servernode == null || Servernode.IsLoggedIn == false) {
                ConnectToServer (null);
            } else {
                CloseConnection ();
            }
        }

        partial void StartSearch (AppKit.NSSearchField sender)
        {
            SearchCertificates (sender.StringValue);
        }

        partial void AddRootCertificate (NSObject sender)
        {
            Servernode.AddRootCertificate ();
        }

        partial void ShowRootCertificate (NSObject sender)
        {
            Servernode.ShowRootCertificate ();
        }

        public void SearchCertificates (String label)
        {
            nint row = splitViewController.MainOutlineView.SelectedRow;
            UIErrorHelper.CheckedExec (delegate() {
                if (row >= (nint)0) {
                    NSObject item = splitViewController.MainOutlineView.ItemAtRow ((int)row);
                    if ((item is VMCACertsNode) || (item is VMCAPersonalCertificatesNode)) {
                        int certState;
                        if (item is VMCACertsNode)
                            certState = (int)(item as VMCACertsNode).Tag;
                        else
                            certState = -1; //private certificates
                        if (certState != -1) {
                            CertificateDetailsListView certView = splitViewController.MainTableView.DataSource as CertificateDetailsListView;
                            X509Certificate2Collection certificateCollection = new X509Certificate2Collection (certView.Entries.ToArray ());
                            X509Certificate2Collection searchResult = certificateCollection.Find (X509FindType.FindBySubjectName, label, false);

                            List<X509Certificate2> certList = new List<X509Certificate2> ();
                            foreach (X509Certificate2 cert in searchResult)
                                certList.Add (cert);
                            splitViewController.MainTableView.DataSource = new CertificateDetailsListView (certList, this.Servernode.ServerDTO, certState);
                        }
                    }
                }
            });
        }

        //strongly typed window accessor
        public new MainWindow Window {
            get {
                return (MainWindow)base.Window;
            }
        }

        //Delegate classes

        class OutlineDelegate : NSOutlineViewDelegate
        {
            MainWindowController ob;

            public OutlineDelegate (MainWindowController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell (NSOutlineView outlineView, NSObject cell,
                                                  NSTableColumn tableColumn, NSObject item)
            {
                try {
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null) {
                        if (item is VMCAServerNode)
                            browserCell.Image = ob.caIcon;
                        else if (item is VMCAPersonalCertificatesNode)
                            browserCell.Image = ob.CertIcon;
                        else if (item is VMCACSRNode)
                            browserCell.Image = ob.CertIcon;
                        else if (item is VMCACertsNode) {
                            if ((int)((item as VMCACertsNode).Tag) == (int)CertificateState.Active)
                                browserCell.Image = ob.ActiveCertIcon;
                            else if ((int)(item as VMCACertsNode).Tag == (int)CertificateState.Expired)
                                browserCell.Image = ob.expiredCertIcon;
                            else if ((int)(item as VMCACertsNode).Tag == (int)CertificateState.Revoked)
                                browserCell.Image = ob.revokedCertIcon;
                        } else if (item is VMCAKeyPairNode)
                            browserCell.Image = ob.KeyPairIcon;
                        else
                            browserCell.Image = ob.CertIcon;
                        browserCell.Leaf = true;
                    }
                } catch (Exception e) {
                    System.Diagnostics.Debug.WriteLine ("Exception in casting : " + e.Message);
                }
            }

            public override void SelectionDidChange (NSNotification notification)
            {
                nint row = ob.splitViewController.MainOutlineView.SelectedRow;
                if (row >= (nint)0) {
                    NSObject item = ob.splitViewController.MainOutlineView.ItemAtRow ((int)row);
                    NSTableView table = ob.splitViewController.MainTableView;

                    ob.RemoveTableColumns ();

                    if (item is VMCACertsNode || item is VMCAPersonalCertificatesNode) { 
                        ob.DisplayRightPaneCertView (item);
                    } else if (item is VMCAKeyPairNode) {
                        ob.DisplayRightPaneKeyPairView (item);
                    } else if (item is VMCACSRNode) {
                        ob.DisplayRightPaneCSRView (item);
                    } else if (item is ChildScopeNode) {
                        NSTableColumn col = new NSTableColumn ("Name");
                        col.HeaderCell.Title = "Name";
                        col.DataCell = new NSBrowserCell ();
                        col.Width = 150;
                        table.AddColumn (col);
                        ChildScopeNode node = item as ChildScopeNode;
                        ob.splitViewController.MainTableView.DataSource = new NodesListView (node.Children, node.ServerDTO);
                    }
                    table.ReloadData ();
                }

            }
        }

        class TableDelegate : NSTableViewDelegate
        {
            MainWindowController ob;

            public TableDelegate (MainWindowController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell (NSTableView tableView, NSObject cell,
                                                  NSTableColumn tableColumn, nint row)
            {
                try {
                    if (tableColumn.Identifier == "Name" || tableColumn.Identifier == "IssuedTo" || tableColumn.Identifier == "CreatedDate") {
                        NSBrowserCell browserCell = cell as NSBrowserCell;
                        if (browserCell != null) {
                            browserCell.Leaf = true;
                            browserCell.Image = ob.genericIcon;
                        }
                    }
                } catch (Exception e) {
                    System.Diagnostics.Debug.WriteLine ("Exception in casting : " + e.Message);
                }
            }
        }
    }
}

