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
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;

using System.Security.Cryptography.X509Certificates;
using System.Collections.Generic;
using CoreGraphics;
using AppKit;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Linq;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Group;

namespace RestSsoAdminSnapIn
{
    public partial class MainWindowController : NSWindowController
    {
        private ServerDto _serverDto;
        private OutlineViewDataSource outlineViewDataSource;
        private SplitViewMMCController splitViewController;
        private OutlineViewNavigationController navigationController;

        public ServerNode Servernode { get; set; }

        public ScopeNode CurrentSelectedNode { get; set; }

        public  string Server { get; set; }

        public MainWindowController (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public MainWindowController (NSCoder coder) : base (coder)
        {
        }

        public MainWindowController () : base ("MainWindow")
        {
            _serverDto = new ServerDto ();
            navigationController = new OutlineViewNavigationController ();
        }

        public MainWindowController (ServerDto serverDto) : base ("MainWindow")
        {
            _serverDto = serverDto;
            Servernode = new ServerNode (_serverDto);
            navigationController = new OutlineViewNavigationController ();
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            ResetToolBarItems ();

            //Load SplitView
            splitViewController = new SplitViewMMCController ();
            splitViewController.MainOutlineView = new TreeView ();
            splitViewController.MainOutlineView.OutlineTableColumn = new NSTableColumn ();
            this.MainCustomView.AddSubview (splitViewController.View);
            var userController = new DefaultViewController ();
            this.DetailedCustomView.AddSubview (userController.View);

            AddToolbarItem.Activated += AddToolbarItem_Activated;
            DeleteToolbarItem.Activated += DeleteToolbarItem_Activated;
            PropertiesToolbarItem.Activated += PropertiesToolbarItem_Activated;
            RefreshToolbarItem.Activated += RefreshToolbarItem_Activated;
            TokenWizardToolbarItem.Activated += TokenWizardToolbarItem_Activated;
            //ADToolbarItem.Activated += ADToolbarItem_Activated;
            ComputerToolbarItem.Activated += ComputerToolbarItem_Activated;
            HttptransportToolbarItem.Activated += HttpTransportToolbarItem_Activated;
			SuperLoggingToolbarItem.Activated += SuperLogToolbarItem_Activated;
            (NSApplication.SharedApplication.Delegate as AppDelegate).OpenConnectionMenuItem.Hidden = true;


            //Notifications for OutlineView and Tableview to reload
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadOutlineView", ReloadOutlineView);
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadTableView", ReloadTableView);
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"RefreshTableView", RefreshTableView);
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"LoggedInSessionExpired", LoggedInSessionExpired);
            NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"RefreshToken", RefreshToken);
			NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"LoginAsUser", LoginAsUser);
        }

        public override void Close ()
        {
            base.Close ();
        }
		void SuperLogToolbarItem_Activated (object sender, EventArgs e)
		{
			TenantNode node;
			if (CurrentSelectedNode is ServerNode)
				node = (CurrentSelectedNode as ServerNode).Children [0] as TenantNode;
			else
				node = CurrentSelectedNode.GetTenantNode () as TenantNode;

			if (node != null) {
				var tenant = node.GetTenant ();
				var form = new SuperLoggingController (){ ServerDto = _serverDto, TenantName = tenant };
				NSApplication.SharedApplication.BeginSheet (form.Window, this.Window, () => {
				});
				try {
					nint result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				} finally {
					Window.EndSheet (form.Window);
					form.Close ();
					form.Dispose ();
				}
			}
		}
        void TokenWizardToolbarItem_Activated (object sender, EventArgs e)
        {	
            var serverNode = CurrentSelectedNode.GetServerNode () as ServerNode;
            if (serverNode != null)
                serverNode.ShowTokenWizard (sender, e);
        }

        void ComputerToolbarItem_Activated (object sender, EventArgs e)
        {
            var serverNode = CurrentSelectedNode.GetServerNode () as ServerNode;
            if (serverNode != null)
                serverNode.OnShowComputers (sender, e);
        }

        void ADToolbarItem_Activated (object sender, EventArgs e)
        {
            var serverNode = CurrentSelectedNode.GetServerNode () as ServerNode;
            if (serverNode != null)
                serverNode.OnShowActiveDirectory (sender, e);
        }

        void RefreshToolbarItem_Activated (object sender, EventArgs e)
        {
            CurrentSelectedNode.Refresh (sender, e);
        }

        void DeleteToolbarItem_Activated (object sender, EventArgs e)
        {
            if (CurrentSelectedNode is ExternalDomainsNode) {
                var node = CurrentSelectedNode as ExternalDomainsNode;
                node.AddNewExternalDomain (sender, e);
            } else {
                var row = (int)splitViewController.MainTableView.SelectedRow;
                if (row >= 0) {
                    var confirm = UIErrorHelper.ShowConfirm ("The selected entry will be permanently deleted. Are you sure?", "Confirmation");
                    if (confirm) {
                        if (CurrentSelectedNode is TenantNode) {
                            var node = CurrentSelectedNode as TenantNode;
                            node.Delete (sender, e);
                            if (!node.Active) {
                                CloseConnection ();
                                ConnectToNewServer ();
                            }
                        } else if (CurrentSelectedNode is UsersNode) {
                            var user = ((UsersDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as UsersNode;
                            node.DeleteUser (user);
                        } else if (CurrentSelectedNode is SolutionUsersNode) {
                            var user = ((SolutionUsersDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as SolutionUsersNode;
                            node.DeleteUser (user);
                        } else if (CurrentSelectedNode is GroupsNode) {
                            var group = ((GroupsDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as GroupsNode;
                            node.DeleteGroup (group);
                        } else if (CurrentSelectedNode is RelyingPartyNode) {
                            var rp = ((RelyingPartyDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as RelyingPartyNode;
                            node.DeleteRelyingParty (rp);
                        } else if (CurrentSelectedNode is OidcClientNode) {
                            var oidc = ((OidcClientDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as OidcClientNode;
                            node.DeleteOidc (oidc);
                        } else if (CurrentSelectedNode is IdentityProvidersNode) {
                            var idp = ((IdentityProvidersDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as IdentityProvidersNode;
                            node.DeleteExternalIdentityProvider (idp);
                        } else if (CurrentSelectedNode is TrustedCertificateNode) {
                            var cert = ((TrustedCertificatesDataSource)splitViewController.MainTableView.DataSource).Entries [row];
                            var node = CurrentSelectedNode as TrustedCertificateNode;
                            node.DeleteCertficateChain (cert);
                        } else if (CurrentSelectedNode is ExternalDomainNode) {
                            var node = CurrentSelectedNode as ExternalDomainNode;
                            node.Delete (this, EventArgs.Empty);
                        }
                    }
                }
            }
        }

        void PropertiesToolbarItem_Activated (object sender, EventArgs e)
        {
            //splitViewController.MainOutlineView.Hidden = !splitViewController.MainOutlineView.Hidden;
            TenantNode node = null;
            if (CurrentSelectedNode is ServerNode)
                node = (CurrentSelectedNode as ServerNode).Children [0] as TenantNode;
            else
                node = CurrentSelectedNode.GetTenantNode () as TenantNode;
            if (node != null)
                node.ShowConfiguration (sender, e);
        }

        void HttpTransportToolbarItem_Activated (object sender, EventArgs e)
        {
            ActionHelper.Execute (delegate {
                var frm = new ShowHttpTransportController ();
                NSApplication.SharedApplication.RunModalForWindow (frm.Window);
            });
        }

        void AddToolbarItem_Activated (object sender, EventArgs e)
        {
            if (CurrentSelectedNode is ServerNode) {
                var node = CurrentSelectedNode as ServerNode;
                node.OnAddNewTenant (sender, e);
            } else if (CurrentSelectedNode is UsersNode) {
                var node = CurrentSelectedNode as UsersNode;
                node.AddNewUser (sender, e);
            } else if (CurrentSelectedNode is SolutionUsersNode) {
                var node = CurrentSelectedNode as SolutionUsersNode;
                node.AddNewUser (sender, e);
            } else if (CurrentSelectedNode is GroupsNode) {
                var node = CurrentSelectedNode as GroupsNode;
                node.AddNewGroup (sender, e);
            } else if (CurrentSelectedNode is ExternalDomainsNode) {
                var node = CurrentSelectedNode as ExternalDomainsNode;
                node.AddNewExternalDomain (sender, e);
            } else if (CurrentSelectedNode is RelyingPartyNode) {
                var node = CurrentSelectedNode as RelyingPartyNode;
                node.AddRelyingParty (sender, e);
            } else if (CurrentSelectedNode is OidcClientNode) {
                var node = CurrentSelectedNode as OidcClientNode;
                node.AddOidcClient (sender, e);
            } else if (CurrentSelectedNode is IdentityProvidersNode) {
                var node = CurrentSelectedNode as IdentityProvidersNode;
                node.AddExternalIdentityProvider (sender, e);
            } else if (CurrentSelectedNode is TrustedCertificateNode) {
                var node = CurrentSelectedNode as TrustedCertificateNode;
                node.AddCertificateChain (sender, e);
            } else if (CurrentSelectedNode is TenantNode) {
                var serverNode = CurrentSelectedNode.GetServerNode ();
                if (serverNode != null) {
                    var node = serverNode as ServerNode;
                    node.OnAddNewTenant (sender, e);
                }
            }
        }

        public new MainWindow Window {
            get { return (MainWindow)base.Window; }
        }

        public void InitialiseViews ()
        {
            ActionHelper.Execute (delegate() {
                CurrentSelectedNode = Servernode;
                outlineViewDataSource = new OutlineViewDataSource (Servernode);
                splitViewController.MainOutlineView.DataSource = outlineViewDataSource;
                splitViewController.MainOutlineView.Activated += OnOutlineViewActivated;
                splitViewController.MainOutlineView.SelectionDidChange += OnOutlineViewActivated;
                splitViewController.MainOutlineView.OutlineTableColumn.DataCell = new NSBrowserCell ();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (Servernode.DisplayName);
                this.TxtLogon.StringValue = "Logged in as " + auth.Login.User + "@" + auth.Login.DomainName;
                InitialiseToolBar ();

                splitViewController.MainOutlineView.Delegate = new OutlineDelegate (this);
                splitViewController.MainTableView.Delegate = new TableDelegate (this);
                CrumbTableView.Hidden = true;
            });
        }

        public void InitialiseToolBar ()
        {
			var enable = Servernode != null && Servernode.Children.Count > 0 && ((TenantNode)Servernode.Children [0]).IsSystemTenant;
			SearchToolbarItem.Active = true;
            ConectToolbarItem.Label = "Disconnect";
            ConectToolbarItem.Active = true;
            BackForwardToolbarItem.Active = true;
            AddToolbarItem.Active = false;
			DeleteToolbarItem.Active = enable;
			PropertiesToolbarItem.Active = enable;
			SuperLoggingToolbarItem.Active = true;
            RefreshToolbarItem.Active = true;
            TokenWizardToolbarItem.Active = true;
			//ADToolbarItem.Active = false;
			ComputerToolbarItem.Active = enable;
            HttptransportToolbarItem.Active = true;

        }

        private void ResetToolBarItems ()
        {
            SearchToolbarItem.Active = false;
            ConectToolbarItem.Label = "Connect";
            ConectToolbarItem.Active = true;
            BackForwardToolbarItem.Active = false;
            AddToolbarItem.Active = false;
            DeleteToolbarItem.Active = false;
            PropertiesToolbarItem.Active = false;
			SuperLoggingToolbarItem.Active = false;
            RefreshToolbarItem.Active = false;
            TokenWizardToolbarItem.Active = false;
            //ADToolbarItem.Active = false;
            ComputerToolbarItem.Active = false;
            HttptransportToolbarItem.Active = false;
        }

		private void ShowLogin (string username)
        {
			var form = new LoginController (){ ServerDto = _serverDto, Username = string.IsNullOrEmpty(username) ? null : username };
            NSApplication.SharedApplication.BeginSheet (form.Window, this.Window, () => {
            });
            try {
                nint result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
                if (result == VMIdentityConstants.DIALOGOK) {
                    Servernode.IsLoggedIn = false;
                    Servernode.LoginDto = form.LoginDto;
                    Servernode.Login ();

					if(Servernode.IsLoggedIn)
					{	
						splitViewController.MainOutlineView.SelectRow(0,true);
					}
                }
            } finally {
                Window.EndSheet (form.Window);
                form.Window.Close ();
                form.Dispose ();
            }
        }

        public void ConnectToServer ()
        {
            ShowLogin (null);
            //Servernode.ShowLogin();
            if (Servernode.IsLoggedIn)
                InitialiseViews ();
            else
                ResetToolBarItems ();
        }

        public void ConnectToNewServer ()
        {
            var form = new AddNewServerController (){ ServerDto = _serverDto };
            NSApplication.SharedApplication.BeginSheet (form.Window, this.Window, () => {
            });
            try {
                nint result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
                if (result == VMIdentityConstants.DIALOGOK) {
                    _serverDto = form.ServerDto;
                    Servernode = new ServerNode (_serverDto);
                    Servernode.IsLoggedIn = false;
                    Servernode.LoginDto = form.LoginDto;
                    Servernode.Login ();
                }
            } finally {
                Window.EndSheet (form.Window);
                form.Close ();
                form.Dispose ();
            }
            if (Servernode != null && Servernode.IsLoggedIn)
                InitialiseViews ();
            else
                ResetToolBarItems ();
        }

        private void CloseConnection ()
        {
            Servernode.Logout ();
            outlineViewDataSource.RootNode.Children.Clear ();
            outlineViewDataSource = null;
            splitViewController.MainOutlineView.DataSource = null;
            splitViewController.MainTableView.DataSource = null;
            this.TxtLogon.StringValue = "";
            splitViewController.MainOutlineView.OutlineTableColumn.HeaderCell.Title = "";
            RemoveTableColumns ();
            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
            ResetToolBarItems ();
            SetDefaultView ();
        }

        partial void SearchText (AppKit.NSSearchField sender)
        {
            ActionHelper.Execute (delegate() {
                var item = CurrentSelectedNode;
                INSTableViewDataSource datasource = null; 
                var columns = new List<ColumnOptions> ();
                RemoveTableColumns ();
                if ((item is UsersNode) || (item is SolutionUsersNode) || (item is GroupsNode)) {
                    var domainName = string.Empty;
                    var tenant = item.GetTenant ();
                    var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
                    domainName = (item is UsersNode) 
						? ((UsersNode)item).DomainName : (item is SolutionUsersNode) 
						? ((SolutionUsersNode)item).DomainName : (item is GroupsNode) 
						? ((GroupsNode)item).DomainName : auth.Login.DomainName;
                    var memberType = (item is UsersNode) ? MemberType.USER : (item is SolutionUsersNode) ? MemberType.SOLUTIONUSER : MemberType.GROUP;
                    var result = SnapInContext.Instance.ServiceGateway.Tenant.Search (_serverDto, tenant, domainName, memberType, SearchType.NAME, auth.Token, sender.StringValue);
                    if (item is UsersNode) {
                        datasource = new UsersDataSource{ Entries = result.Users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                            //new ColumnOptions{ DisplayName = "First Name", Id = "FirstName", Width = 100 },
                            //new ColumnOptions{ DisplayName = "Last Name", Id = "LastName", Width = 100 },
                            //new ColumnOptions{ DisplayName = "Email", Id = "Email", Width = 80 },
                            new ColumnOptions{ DisplayName = "Description", Id = "Description", Width = 350 }
                        };
                    } else if (item is SolutionUsersNode) {
                        datasource = new SolutionUsersDataSource{ Entries = result.SolutionUsers };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                            new ColumnOptions{ DisplayName = "Disabled", Id = "Disabled", Width = 60 },
                            new ColumnOptions{ DisplayName = "Description", Id = "Description", Width = 350 }
                        };
                    } else if (item is GroupsNode) {
                        datasource = new GroupsDataSource{ Entries = result.Groups };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                            //new ColumnOptions{ DisplayName = "Domain", Id = "Domain", Width = 150 },
                            new ColumnOptions{ DisplayName = "Description", Id = "Description", Width = 350 }
                        };
                    } 
                } else if (item is RelyingPartyNode) {
                    var source = splitViewController.MainTableView.DataSource as RelyingPartyDataSource;
                    var nodes = source.Entries.FindAll (x => x.Name.Contains (sender.StringValue));
                    datasource = new RelyingPartyDataSource{ Entries = nodes };
                    columns = new List<ColumnOptions> { 
                        new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                        new ColumnOptions{ DisplayName = "Url", Id = "Url", Width = 250 }
                    };
                } else if (item is TrustedCertificateNode) {
                    var source = splitViewController.MainTableView.DataSource as TrustedCertificatesDataSource;
                    var nodes = source.Entries.FindAll (x => x.Chain.Contains (sender.StringValue));
                    datasource = new TrustedCertificatesDataSource{ Entries = nodes };
                    columns = new List<ColumnOptions> { 
                        new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 80 },
                        new ColumnOptions{ DisplayName = "Status", Id = "Status", Width = 40 },
                        new ColumnOptions{ DisplayName = "Issued By", Id = "IssuedBy", Width = 100 },
                        new ColumnOptions{ DisplayName = "Issued On", Id = "IssuedOn", Width = 80 },
                        new ColumnOptions{ DisplayName = "Expiration", Id = "Expiration", Width = 80 },
                        new ColumnOptions{ DisplayName = "Purpose", Id = "Purpose", Width = 150 },
                    };
                } else if (item is IdentityProvidersNode) {
                    var source = splitViewController.MainTableView.DataSource as IdentityProvidersDataSource;
                    var nodes = source.Entries.FindAll (x => x.EntityID.Contains (sender.StringValue));
                    datasource = new IdentityProvidersDataSource{ Entries = nodes };
                    columns = new List<ColumnOptions> { 
                        new ColumnOptions{ DisplayName = "Entity Id", Id = "Name", Width = 350 },
                        new ColumnOptions{ DisplayName = "Jit", Id = "Jit", Width = 60 }
                    };
                } else if (item is OidcClientNode) {
                    var source = splitViewController.MainTableView.DataSource as OidcClientDataSource;
                    var nodes = source.Entries.FindAll (x => x.ClientId.Contains (sender.StringValue));
                    datasource = new OidcClientDataSource{ Entries = nodes };
                    columns = new List<ColumnOptions> { 
                        new ColumnOptions{ DisplayName = "Client Id", Id = "Name", Width = 350 },
                        new ColumnOptions{ DisplayName = "Certificate DN", Id = "CertificateDN", Width = 350 }
                    };
                } else {
                    var source = splitViewController.MainTableView.DataSource as NodesDefaultDataSource;
                    var nodes = source.Entries.FindAll (x => x.DisplayName.Contains (sender.StringValue));
                    datasource = new NodesDefaultDataSource (){ Entries = nodes };
                    columns = new List<ColumnOptions> { new ColumnOptions {
                            DisplayName = "Name",
                            Id = "Name",
                            Width = 350,
                        }
                    };
                }
                foreach (var col in ListViewHelper.ToNSTableColumns (columns)) {
                    splitViewController.MainTableView.AddColumn (col);
                }
                splitViewController.MainTableView.DataSource = datasource;
            });
        }

        void RemoveTableColumns ()
        {
            while (splitViewController.MainTableView.ColumnCount > 0) {
                splitViewController.MainTableView.RemoveColumn (splitViewController.MainTableView.TableColumns () [0]);
            }
        }

        void SetDefaultView ()
        {
            AppKit.NSView oldView = null;
            var controller = new DefaultViewController ();
            if (DetailedCustomView.Subviews.Count () > 0) {
                oldView = DetailedCustomView.Subviews [0];
            }
            if (oldView != null)
                DetailedCustomView.ReplaceSubviewWith (oldView, controller.View);
            else
                DetailedCustomView.AddSubview (controller.View);
        }

        // SplitView Events
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

        public void RefreshToken (NSNotification notification)
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (Servernode.DisplayName);
            if (auth != null)
                auth.Refresh ();
        }

        public void LoggedInSessionExpired (NSNotification notification)
        {
            ShowLogin (null);
        }

		public void LoginAsUser(NSNotification notification){
			
			var row = (int)splitViewController.MainTableView.SelectedRow;
			if (row >= 0) {
				ActionHelper.Execute (delegate() {
					var user = ((UsersDataSource)splitViewController.MainTableView.DataSource).Entries [row];
					ShowLogin (user.Name);
				});
			}
		}

        public void RefreshTableView (NSNotification notification)
        {
            splitViewController.MainOutlineView.Delegate.SelectionDidChange (notification);
        }

        // ToolBar Events
        partial void ConnectServerAction (Foundation.NSObject sender)
        {
            if (Servernode == null || (Servernode != null && Servernode.IsLoggedIn == false)) {
                //ConnectToServer ();
                ConnectToNewServer ();
            } else {
                CloseConnection ();
            }
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

        private void GotoPreviousAction ()
        {
            splitViewController.MainOutlineView.DeselectAll (this);
            nint row = (nint)navigationController.GetPreviousSelectedRow ();
            splitViewController.MainOutlineView.SelectRow (row, true);
        }

        public void ToggleToolbarState (ActivatableToolBarItem toolbarItem, bool state)
        {
            toolbarItem.Active = state;
        }

        private void GotoNextAction ()
        {
            splitViewController.MainOutlineView.DeselectAll (this);
            nint row = (nint)navigationController.GetForwardSelectedRow ();
            splitViewController.MainOutlineView.SelectRow (row, true);
        }


        public override void WindowDidLoad ()
        {
            base.WindowDidLoad ();
            ConnectToNewServer ();
        }


        //Delegate classes
        class OutlineDelegate : NSOutlineViewDelegate
        {
            private MainWindowController _controller;

            public OutlineDelegate (MainWindowController controller)
            {
                this._controller = controller;
            }

            public override void WillDisplayCell (NSOutlineView outlineView, NSObject cell,
                                                  NSTableColumn tableColumn, NSObject item)
            {
                try {
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null) {
                        browserCell.Leaf = true;

                        if (item is ServerNode)
                            browserCell.Image = NSImage.ImageNamed ("Disconnect_64.png");
                        else if (item is TenantNode)
                            browserCell.Image = NSImage.ImageNamed ("tenant.png");
                        else if (item is UsersNode)
                            browserCell.Image = NSImage.ImageNamed ("NSUser");
                        else if (item is SolutionUsersNode)
                            browserCell.Image = NSImage.ImageNamed ("NSUserGuest");
                        else if (item is GroupsNode)
                            browserCell.Image = NSImage.ImageNamed ("NSUserGroup");
                        else if (item is IdentitySourcesNode)
                            browserCell.Image = NSImage.ImageNamed ("system.png");
                        else if (item is IdentitySourceNode)
                            browserCell.Image = NSImage.ImageNamed ("system.png");
                        else if (item is RelyingPartyNode)
                            browserCell.Image = NSImage.ImageNamed ("provider.png");
                        else if (item is IdentityProvidersNode)
                            browserCell.Image = NSImage.ImageNamed ("provider.png");
                        else if (item is ExternalDomainsNode)
                            browserCell.Image = NSImage.ImageNamed ("Welcome.png");
                        else if (item is ExternalDomainNode)
                            browserCell.Image = NSImage.ImageNamed ("Welcome.png");
                        else if (item is ServerCertificatesNode)
                            browserCell.Image = NSImage.ImageNamed ("system.png");
                        else if (item is TrustedCertificateNode)
                            browserCell.Image = NSImage.ImageNamed ("certificate.png");
                        else if (item is OidcClientNode)
                            browserCell.Image = NSImage.ImageNamed ("provider.png");
                        else if (item is UsersAndGroupsNode)
                            browserCell.Image = NSImage.ImageNamed ("provider.png");
                        browserCell.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
                    }
                } catch (Exception) {
                    //do nothing
                }
            }

            public override void SelectionDidChange (NSNotification notification)
            {
                nint row = _controller.splitViewController.MainOutlineView.SelectedRow;
                if (row >= (nint)0) {
                    NSObject item = _controller.splitViewController.MainOutlineView.ItemAtRow ((int)row);
                    _controller.SetDefaultView ();
                    //NSTableView table = _controller.splitViewController.MainTableView;
                    _controller.RemoveTableColumns ();
                    List<ColumnOptions> columns = new List<ColumnOptions> ();
                    NSTableViewDataSource datasource;
                    _controller.CurrentSelectedNode = item as ScopeNode;
                    if (item is TrustedCertificateNode) {
                        _controller.AddToolbarItem.Label = "Add Cert Chain";
                        _controller.AddToolbarItem.Active = true;
                        _controller.DeleteToolbarItem.Label = "Delete Cert Chain";
                        _controller.DeleteToolbarItem.Active = true;
                        var trustedCertificateNode = item as TrustedCertificateNode;
                        var certificates = trustedCertificateNode.GetCertificates ();
                        datasource = new TrustedCertificatesDataSource{ Entries = certificates };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 100 },
                            new ColumnOptions{ DisplayName = "Status", Id = "Status", Width = 40 },
                            new ColumnOptions{ DisplayName = "Issued By", Id = "IssuedBy", Width = 150 },
                            new ColumnOptions{ DisplayName = "Issued On", Id = "IssuedOn", Width = 60 },
                            new ColumnOptions{ DisplayName = "Expiration", Id = "Expiration", Width = 60 },
                            new ColumnOptions{ DisplayName = "Purpose", Id = "Purpose", Width = 250 },
                        };
                    } else if (item is UsersNode) {
                        var node = item as UsersNode;
                        _controller.AddToolbarItem.Label = "Add User";
                        _controller.AddToolbarItem.Active = node.IsSystemDomain;
                        _controller.DeleteToolbarItem.Label = "Delete User";
                        _controller.DeleteToolbarItem.Active = node.IsSystemDomain;
                        var users = node.GetUsers (string.Empty);
                        datasource = new UsersDataSource{ Entries = users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                            //new ColumnOptions{ DisplayName = "First Name", Id = "FirstName", Width = 100 },
                            //new ColumnOptions{ DisplayName = "Last Name", Id = "LastName", Width = 100 },
                            //new ColumnOptions{ DisplayName = "Email", Id = "Email", Width = 80 },
                            new ColumnOptions{ DisplayName = "Description", Id = "Description", Width = 350 }
                        };
                    } else if (item is SolutionUsersNode) {
                        var node = item as SolutionUsersNode;
                        _controller.AddToolbarItem.Label = "Add Solution User";
                        _controller.AddToolbarItem.Active = node.IsSystemDomain;
                        _controller.DeleteToolbarItem.Label = "Delete Solution User";
                        _controller.DeleteToolbarItem.Active = node.IsSystemDomain;
                        var users = node.GetUsers (string.Empty);
                        datasource = new SolutionUsersDataSource{ Entries = users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
							new ColumnOptions{ DisplayName = "Disabled", Id = "Disabled", Width = 60 },
                            new ColumnOptions{ DisplayName = "Description", Id = "Description", Width = 350 }
                            
                        };
                    } else if (item is GroupsNode) {
                        var node = item as GroupsNode;
                        _controller.AddToolbarItem.Label = "Add Group";
                        _controller.AddToolbarItem.Active = node.IsSystemDomain;
                        _controller.DeleteToolbarItem.Label = "Delete Group";
                        _controller.DeleteToolbarItem.Active = node.IsSystemDomain;
                        var users = node.GetGroups (string.Empty);
                        datasource = new GroupsDataSource{ Entries = users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                            //new ColumnOptions{ DisplayName = "Domain", Id = "Domain", Width = 100 },
                            new ColumnOptions{ DisplayName = "Description", Id = "Description", Width = 350 }
                        };
                    } else if (item is RelyingPartyNode) {
                        _controller.AddToolbarItem.Label = "Add Relying Party";
                        _controller.AddToolbarItem.Active = true;
                        _controller.DeleteToolbarItem.Label = "Delete Relying Party";
                        _controller.DeleteToolbarItem.Active = true;
                        var node = item as RelyingPartyNode;
                        var users = node.GetRelyingParty ();
                        datasource = new RelyingPartyDataSource{ Entries = users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Name", Id = "Name", Width = 350 },
                            new ColumnOptions{ DisplayName = "Url", Id = "Url", Width = 150 }
                        };
                    } else if (item is IdentityProvidersNode) {
                        _controller.AddToolbarItem.Label = "Add External IDP";
                        _controller.AddToolbarItem.Active = true;
                        _controller.DeleteToolbarItem.Label = "Delete External IDP";
                        _controller.DeleteToolbarItem.Active = true;
                        var node = item as IdentityProvidersNode;
                        var users = node.GetIdentityProviders ();
                        datasource = new IdentityProvidersDataSource{ Entries = users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Entity Id", Id = "Name", Width = 350 },
                            new ColumnOptions{ DisplayName = "Jit", Id = "Jit", Width = 60 }
                        };
                    } else if (item is OidcClientNode) {
                        _controller.AddToolbarItem.Label = "Add OIDC Client";
                        _controller.AddToolbarItem.Active = true;
                        _controller.DeleteToolbarItem.Label = "Delete OIDC Client";
                        _controller.DeleteToolbarItem.Active = true;
                        var node = item as OidcClientNode;
                        var users = node.GetOidcClients ();
                        datasource = new OidcClientDataSource{ Entries = users };
                        columns = new List<ColumnOptions> { 
                            new ColumnOptions{ DisplayName = "Client Id", Id = "Name", Width = 350 },
                            new ColumnOptions{ DisplayName = "Certificate DN", Id = "CertificateDN", Width = 150 }
                        };
                    } else if (item is ServerNode) {
                        _controller.AddToolbarItem.Label = "Add Server";
                        _controller.AddToolbarItem.Active = false;
                        _controller.DeleteToolbarItem.Label = "Delete Server";
                        _controller.DeleteToolbarItem.Active = false;
                        ScopeNode node = item as ScopeNode;
                        datasource = new NodesDefaultDataSource{ Entries = node.Children };
                        columns = new List<ColumnOptions> { new ColumnOptions {
                                DisplayName = "Name",
                                Id = "Name",
                                Width = 350,
                            }
                        };
                    } else if (item is ExternalDomainsNode) {
                        _controller.AddToolbarItem.Label = "External Domain";
                        _controller.AddToolbarItem.Active = true;
                        _controller.DeleteToolbarItem.Label = "Delete External Domain";
                        _controller.DeleteToolbarItem.Active = false;
                        ScopeNode node = item as ScopeNode;
                        datasource = new NodesDefaultDataSource{ Entries = node.Children };
                        columns = new List<ColumnOptions> { new ColumnOptions {
                                DisplayName = "Name",
                                Id = "Name",
                                Width = 350,
                            }
                        };
                    } else if (item is ExternalDomainNode) {
                        _controller.AddToolbarItem.Label = "External Domain";
                        _controller.AddToolbarItem.Active = true;
                        _controller.DeleteToolbarItem.Label = "Delete External Domain";
                        _controller.DeleteToolbarItem.Active = true;
                        ScopeNode node = item as ScopeNode;
                        datasource = new NodesDefaultDataSource{ Entries = node.Children };
                        columns = new List<ColumnOptions> { new ColumnOptions {
                                DisplayName = "Name",
                                Id = "Name",
                                Width = 350,
                            }
                        };
                    } else if (item is TenantNode) {
                        _controller.AddToolbarItem.Label = "Add Tenant";
                        _controller.AddToolbarItem.Active = ((TenantNode)item).IsSystemTenant;
                        _controller.DeleteToolbarItem.Label = "Delete Tenant";
                        _controller.DeleteToolbarItem.Active = true;
                        _controller.PropertiesToolbarItem.Label = "Tenant Configuration";
                        ScopeNode node = item as ScopeNode;
                        datasource = new NodesDefaultDataSource{ Entries = node.Children };
                        columns = new List<ColumnOptions> { new ColumnOptions {
                                DisplayName = "Name",
                                Id = "Name",
                                Width = 350,
                            }
                        };
                    } else {
                        _controller.AddToolbarItem.Active = false;
                        _controller.DeleteToolbarItem.Active = false;
                        ScopeNode node = item as ScopeNode;
                        datasource = new NodesDefaultDataSource{ Entries = node.Children };
                        columns = new List<ColumnOptions> { new ColumnOptions {
                                DisplayName = "Name",
                                Id = "Name",
                                Width = 350,
                            }
                        };
                    }

                    foreach (var col in ListViewHelper.ToNSTableColumns (columns)) {
                        _controller.splitViewController.MainTableView.AddColumn (col);
                    }
                    _controller.splitViewController.MainTableView.DataSource = datasource;
                    _controller.splitViewController.MainTableView.ReloadData ();
                    var previous = _controller.splitViewController.MainTableView.SelectedRow;
                    if (_controller.splitViewController.MainTableView.RowCount > 0)
                        _controller.splitViewController.MainTableView.SelectRow (0, false);
                    if (previous == 0)
                        _controller.splitViewController.MainTableView.Delegate.SelectionDidChange (notification);
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
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null) {
                        browserCell.Leaf = true;
                        if (tableColumn.Identifier == "Name") {
                            if (ob.CurrentSelectedNode is UsersNode) {
                                browserCell.Image = NSImage.ImageNamed ("NSUser");
                            } else if (ob.CurrentSelectedNode is SolutionUsersNode) {
                                browserCell.Image = NSImage.ImageNamed ("NSUserGuest.png");
                            } else if (ob.CurrentSelectedNode is GroupsNode) {
                                browserCell.Image = NSImage.ImageNamed ("NSUserGroup.png");
                            } else if (ob.CurrentSelectedNode is RelyingPartyNode) {
                                browserCell.Image = NSImage.ImageNamed ("provider.png");
                            } else if (ob.CurrentSelectedNode is OidcClientNode) {
                                browserCell.Image = NSImage.ImageNamed ("provider.png");
                            } else if (ob.CurrentSelectedNode is TrustedCertificateNode) {
                                browserCell.Image = NSImage.ImageNamed ("certificate.png");
                            } else if (ob.CurrentSelectedNode is IdentityProvidersNode) {
                                browserCell.Image = NSImage.ImageNamed ("provider.png");
                            }
                            browserCell.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
                        }
                    }
                } catch (Exception) {
                    // do nothing
                }
            }

            public override void SelectionDidChange (NSNotification notification)
			{
				ActionHelper.Execute (delegate() {
					var row = (int)ob.splitViewController.MainTableView.SelectedRow;
					AppKit.NSViewController controller = null;
					if (row >= 0) {
						ActionHelper.Execute (delegate() {
							if (ob.CurrentSelectedNode is UsersNode) {
								var node = ob.CurrentSelectedNode as UsersNode;
								var tenant = node.GetTenant ();
								var user = ((UsersDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								var groups = new List<GroupDto>();
								try
								{
									var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ob._serverDto.ServerName);
									groups = SnapInContext.Instance.ServiceGateway.User.GetUserGroups (ob._serverDto, tenant, user, auth.Token);
									user.ActAsUsers = groups != null && groups.Exists (x => x.GroupName == "ActAsUsers" && x.GroupDomain == user.Domain);
									user.IsIdpAdmin = groups != null && groups.Exists (x => x.GroupName == "IdpProvisioningAdmin" && x.GroupDomain == user.Domain);
									var isAdmin = groups != null && groups.Exists (x => x.GroupName == "Administrators" && x.GroupDomain == user.Domain);
									var isUser = groups != null && groups.Exists (x => x.GroupName == "Users" && x.GroupDomain == user.Domain);
									user.Role = isAdmin ? UserRole.Administrator : isUser ? UserRole.RegularUser : UserRole.GuestUser;
								}
								catch(Exception exc)
								{
									user.ActAsUsers = false;
									user.IsIdpAdmin = false;
									user.Role = UserRole.GuestUser;
								}
								controller = new UserDetailsViewController () {
									UserDtoOriginal = user,
									GroupsOriginal = groups,
									ServerDto = ob._serverDto,
									TenantName = tenant,
									IsSystemDomain = node.IsSystemDomain
								};

							} else if (ob.CurrentSelectedNode is SolutionUsersNode) {
								var node = ob.CurrentSelectedNode as SolutionUsersNode;
								var tenant = node.GetTenant ();
								var user = ((SolutionUsersDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								controller = new SolutionUserDetailsViewController () {
									SolutionUserDtoOriginal = user,
									ServerDto = ob._serverDto,
									TenantName = tenant,
									IsSystemDomain = node.IsSystemDomain
								};
							} else if (ob.CurrentSelectedNode is GroupsNode) {
								var node = ob.CurrentSelectedNode as GroupsNode;
								var tenant = node.GetTenant ();
								var user = ((GroupsDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ob._serverDto.ServerName);
								var memberInfo = new GroupMembershipDto();
								var userInfo = SnapInContext.Instance.ServiceGateway.Group.GetMembers (ob._serverDto, tenant, user, GroupMemberType.USER, auth.Token);
								memberInfo.Users = userInfo.Users == null ? new List<UserDto> () : new List<UserDto> (userInfo.Users);
								userInfo = SnapInContext.Instance.ServiceGateway.Group.GetMembers (ob._serverDto, tenant, user, GroupMemberType.GROUP, auth.Token);
								memberInfo.Groups = userInfo.Groups == null ? new List<GroupDto> () : new List<GroupDto> (userInfo.Groups);

								controller = new GroupDetailsViewController () {
									GroupDtoOriginal = user,
									GroupsMembershipDtoOriginal = memberInfo,
									ServerDto = ob._serverDto,
									TenantName = tenant,
									IsSystemDomain = node.IsSystemDomain
								};
							} else if (ob.CurrentSelectedNode is TrustedCertificateNode) {
								var certificate = ((TrustedCertificatesDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								controller = new CertificateDetailsViewController () {
									CertificateDto = certificate
								};
							} else if (ob.CurrentSelectedNode is RelyingPartyNode) {
								var node = ob.CurrentSelectedNode as RelyingPartyNode;
								var tenant = node.GetTenant ();
								var rp = ((RelyingPartyDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								controller = new RelyingPartyDetailsViewController () { 
									RelyingPartyDtoOriginal = rp, 
									ServerDto = ob._serverDto,
									TenantName = tenant
								};
							} else if (ob.CurrentSelectedNode is OidcClientNode) {
								var node = ob.CurrentSelectedNode as OidcClientNode;
								var rp = ((OidcClientDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								var tenant = node.GetTenant ();
								controller = new OidcClientDetailsViewController () {
									OidcClientDtoOriginal = rp,
									ServerDto = ob._serverDto,
									TenantName = tenant
								};
							} else if (ob.CurrentSelectedNode is IdentityProvidersNode) {
								var node = ob.CurrentSelectedNode as IdentityProvidersNode;
								var rp = ((IdentityProvidersDataSource)ob.splitViewController.MainTableView.DataSource).Entries [row];
								var rpLatest = node.GetIdentityProvider (rp);
								var tenant = node.GetTenant ();
								controller = new ExtenalIdpDetailsViewController () {
									ExternalIdentityProviderDto = rpLatest,
									ServerDto = ob._serverDto,
									TenantName = tenant
								};
							} else {
								controller = new DefaultViewController ();
							}
						});
						if (controller == null)
							controller = new DefaultViewController ();

						AppKit.NSView oldView = null;
						if (ob.DetailedCustomView.Subviews.Count () > 0) {
							oldView = ob.DetailedCustomView.Subviews [0];
						}
						if (oldView != null)
							ob.DetailedCustomView.ReplaceSubviewWith (oldView, controller.View);
						else
							ob.DetailedCustomView.AddSubview (controller.View);
					
					}
				});
			}
        }
    }
}
