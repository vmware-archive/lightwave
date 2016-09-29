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
using System.Linq;
using System.Collections.Generic;
using System.Net.NetworkInformation;
using System.Threading.Tasks;
using AppKit;
using Foundation;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.DataSources;
using VMPSCHighAvailability.Nodes;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.Service;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Main window controller for the tool. Encapsutlates all the actions for Main window.
	/// </summary>
	public partial class VMPSCHighAvailabilityMainWindowController : MainWindowCommonController
	{
		public NSImage[] CachedImages{ get; private set;}

		/// <summary>
		/// The split view controller.
		/// </summary>
		private SplitCustomViewController controller;

		/// <summary>
		/// The root node.
		/// </summary>
		private GlobalTopologyNode _rootNode;

		/// <summary>
		/// The management dto.
		/// </summary>
		private ManagementDto _managementDto;

		/// <summary>
		/// Holds the server Dto for the Main window.
		/// </summary>
		private ServerDto _serverDto;

		/// <summary>
		/// The state of the server.
		/// </summary>
		private ServerState _serverState;

		private IPscHighAvailabilityService _service;

		/// <summary>
		/// Is user logged in.
		/// </summary>
		private bool _isLoggedIn;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.VMPSCHighAvailabilityMainWindowController"/> class.
		/// </summary>
		/// <param name="handle">Handle.</param>
		public VMPSCHighAvailabilityMainWindowController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.VMPSCHighAvailabilityMainWindowController"/> class.
		/// </summary>
		/// <param name="coder">Coder.</param>
		public VMPSCHighAvailabilityMainWindowController (NSCoder coder) : base (coder)
		{
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.VMPSCHighAvailabilityMainWindowController"/> class.
		/// </summary>
		public VMPSCHighAvailabilityMainWindowController () : base ("MainWindowCommon")
		{
		}

		/// <summary>
		/// Awakes from nib.
		/// </summary>
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			_service = new PscHighAvailabilityService (PscHighAvailabilityAppEnvironment.Instance.Logger,PscHighAvailabilityAppEnvironment.Instance);
			this.Window.MinSize = new CoreGraphics.CGSize (){Height = 607, Width= 820};
			this.Window.Title = Constants.SuiteName + " " + Constants.ToolName;
			ServerToolBarItem.Activated += OnServerToolBarItem_Activated;

			NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadOutlineView", ReloadOutlineView);
			NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"ReloadTableView", ReloadTableView);
			RefreshToolBarItem.Label = "Refresh Topology";
			RefreshToolBarItem.Activated += (object sender, EventArgs e) =>  {
				TopologyRefresh ();
			};
			CacheImages ();
		}

		/// <summary>
		/// Caches the images.
		/// </summary>
		private void CacheImages ()
		{
			CachedImages = new NSImage[(int)ImageIndex.None];
			CachedImages[(int)ImageIndex.Global] = NSImage.ImageNamed (Constants.GlobalNodeImage);
			CachedImages[(int)ImageIndex.Site] = NSImage.ImageNamed (Constants.SiteNodeImage);
			CachedImages[(int)ImageIndex.InfraGroup] = NSImage.ImageNamed (Constants.InfrastructureGroupNodeImage);
			CachedImages[(int)ImageIndex.ManagementGroup] = NSImage.ImageNamed (Constants.ManagementGroupNodeImage);
			CachedImages[(int)ImageIndex.Infrastructure] = NSImage.ImageNamed (Constants.InfrastructureNodeImage);
			CachedImages[(int)ImageIndex.Management] = NSImage.ImageNamed (Constants.ManagementNodeImage);
			CachedImages[(int)ImageIndex.Service] = NSImage.ImageNamed (Constants.ServiceImage);
			CachedImages[(int)ImageIndex.Server] = NSImage.ImageNamed (Constants.GlobalNodeImage);

			foreach (var item in CachedImages)
				item.Size = new CoreGraphics.CGSize (16, 16);
		}

		/// <summary>
		/// Gets the window.
		/// </summary>
		/// <value>The window.</value>
		public new MainWindowCommon Window {
			get { return (MainWindowCommon)base.Window; }
		}

		/// <summary>
		/// Windows the did load.
		/// </summary>
		public override void WindowDidLoad ()
		{
			base.WindowDidLoad ();
			ConnectToServer ();
		}

		/// <summary>
		/// Gets the server dto.
		/// </summary>
		/// <returns>The server dto.</returns>
		/// <param name="controller">Controller.</param>
		private ServerDto GetServerDto (LoginWindowController controller)
		{
			var serverDto = new ServerDto {
				Server = controller.Server,
				UserName = controller.UserName,
				Password = controller.Password,
				DomainName = controller.DomainName,
				Upn = controller.Upn
			};
			return serverDto;
		}

		/// <summary>
		/// Connects to server.
		/// </summary>
		/// <returns>The to server.</returns>
		/// <param name="server">Server.</param>
		public ServerDto ConnectToServer (string server)
		{
			var form = new LoginWindowController (){ Server = server };
			var serverDto = new ServerDto{ Server = server };
			NSApplication.SharedApplication.BeginSheet (form.Window, this.Window, () => {
			});
			try {
				nint result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					serverDto = new ServerDto {
						Server = form.Server,
						UserName = form.UserName,
						Password = form.Password,
						DomainName = form.DomainName,
						Upn = form.Upn
					};
				}
			} finally {
				Window.EndSheet (form.Window);
				form.Close ();
				form.Dispose ();
			}
			return serverDto;
		}

		/// <summary>
		/// Connects to server.
		/// </summary>
		public async void ConnectToServer ()
		{
			_serverState = ServerState.Disconnected;
			ProgressWindowController pwc = new ProgressWindowController ();
			IntPtr session = new IntPtr (0);
			var servers = PscHighAvailabilityAppEnvironment.Instance.LocalData.GetServerArray ();
			var lwc = new LoginWindowController (servers);
			NSApplication.SharedApplication.BeginSheet (lwc.Window, this.Window, () => {
			});
			nint result = NSApplication.SharedApplication.RunModalForWindow (lwc.Window);
			try {
				if (result == (nint)VMIdentityConstants.DIALOGOK) {
					_serverDto = GetServerDto (lwc);
					if(PingHost(_serverDto.Server))
					{
						NSApplication.SharedApplication.BeginSheet (pwc.Window, this.Window as NSWindow, () => {
						});
						session = NSApplication.SharedApplication.BeginModalSession (pwc.Window);

						var task = new Task(LoginToServer);
						task.Start();
						if (task == await Task.WhenAny (task, Task.Delay (VMIdentityConstants.ServerTimeoutInSeconds * Constants.MilliSecsMultiplier)))
						{	
							await task;
							if (!_isLoggedIn)
							{
								UIErrorHelper.ShowAlert (VMIdentityConstants.SERVER_CONNECT_ERROR, VMIdentityConstants.SERVER_CONNECT_ERROR);
							}
							else
							{
								PscHighAvailabilityAppEnvironment.Instance.LocalData.AddServer (lwc.Server);
								this.LoggedInLabel.StringValue = _serverDto.Upn;
								_serverState = ServerState.Connected;
							}
						}
						else
						{
							UIErrorHelper.ShowAlert(VMIdentityConstants.SERVER_TIMED_OUT, VMIdentityConstants.SERVER_TIMED_OUT);
						}
					}
					else
					{
						UIErrorHelper.ShowAlert(VMIdentityConstants.HOST_OR_IP_ADDRESS_NOT_REACHABLE, VMIdentityConstants.HOST_NOT_REACHABLE);
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

			SetConnectToolbar(_serverState);
			if (_isLoggedIn) {
				Initialize ();
				AsyncRefresh ();
			}
		}

		/// <summary>
		/// Loads the split view.
		/// </summary>
		private void Initialize()
		{
			
			var c = new DefaultEmptyController ();
			controller = new SplitCustomViewController () {
				TreeView = new NSOutlineView(),
				DetailViewController = c
			};

			var cg = new CoreGraphics.CGSize ();
			cg.Height = ContainerView.Frame.Height;
			cg.Width = ContainerView.Frame.Width;
			controller.View.SetFrameSize (cg);

			this.ContainerView.AddSubview(controller.View);
		}

		/// <summary>
		/// Reloads the outline view.
		/// </summary>
		/// <param name="notification">Notification.</param>
		public void ReloadOutlineView (NSNotification notification)
		{
			controller.TreeView.ReloadData ();
		}

		/// <summary>
		/// Reloads the table view.
		/// </summary>
		/// <param name="notification">Notification.</param>
		public void ReloadTableView (NSNotification notification)
		{
			RefreshTableViewsBasedOnSelection (controller.TreeView.SelectedRow);
		}

		/// <summary>
		/// Reloads the table view.
		/// </summary>
		/// <param name="notification">Notification.</param>
		public void ReloadTableView ()
		{
			RefreshTableViewsBasedOnSelection (controller.TreeView.SelectedRow);
		}

		/// <summary>
		/// Reloads the main table.
		/// </summary>
		/// <param name="tableView">Table view.</param>
		/// <param name="columnNames">Column names.</param>
		/// <param name="filteredHosts">Filtered hosts.</param>
		private void ReloadMainTable (NSTableView tableView, List<ColumnOptions> columnNames, List<NodeDto> filteredHosts)
		{
			var columns = NSTableColumnHelper.ToNSTableColumns (columnNames);
			while (tableView.ColumnCount > 0) {
				tableView.RemoveColumn (tableView.TableColumns () [0]);
			}
			foreach (var column in columns) {
				tableView.AddColumn (column);
			}
			tableView.DataSource = new NodeDataSource (filteredHosts);
			tableView.ReloadData ();
		}

		/// <summary>
		/// Gets the host column options for main table.
		/// </summary>
		/// <returns>The host column options for main table.</returns>
		private List<ColumnOptions> GetHostColumnOptionsForMainTable()
		{
			return new List<ColumnOptions> {
				new ColumnOptions{
					Id = Constants.TableColumnIconId,
					DisplayName = string.Empty,
					DisplayOrder = 10,
					Width = 20,
					Type = ColumnType.Browser
				},
				new ColumnOptions {
					Id = Constants.PscTableColumnNameId,
					DisplayName = Constants.PscTableColumnNameId,
					DisplayOrder = 20,
					Width = 300,
					Type = ColumnType.Text
				},
				new ColumnOptions {
					Id = Constants.PscTableColumnStatusId,
					DisplayName = Constants.PscTableColumnStatusId,
					DisplayOrder = 30,
					Width = 80,
					Type = ColumnType.Text
				},
			};
		}

		/// <summary>
		/// Gets the global view column options for main table.
		/// </summary>
		/// <returns>The global view column options for main table.</returns>
		private List<ColumnOptions> GetGlobalViewColumnOptionsForMainTable()
		{
			var options = GetHostColumnOptionsForMainTable ();
			var columnOption = new ColumnOptions {
				Id = Constants.PscTableColumnSitenameId,
				DisplayName = Constants.PscTableColumnSitenameId,
				DisplayOrder = 11,
				Width = 160,
				Type = ColumnType.Text
			};
			options.Add (columnOption);
			return options;
		}

		/// <summary>
		/// Gets the infra node detail view column options for main table.
		/// </summary>
		/// <returns>The infra node detail view column options for main table.</returns>
		private List<ColumnOptions> GetInfraNodeDetailViewColumnOptionsForMainTable()
		{
			return new List<ColumnOptions> {
				new ColumnOptions{ 
					Id = Constants.TableColumnIconId,
					DisplayName = string.Empty,
					DisplayOrder = 10,
					Width = 20,
					Type = ColumnType.Browser
				},
				new ColumnOptions {
					Id = Constants.PscTableColumnNameId,
					DisplayName = Constants.PscTableColumnNameId,
					DisplayOrder = 15,
					Width = 320,
					Type = ColumnType.Text
				},
				new ColumnOptions {
					Id = Constants.ServiceTableColumnNameId,
					DisplayName = Constants.ServiceTableColumnNameId,
					DisplayOrder = 20,
					Width = 100,
					Type = ColumnType.Text
				},
				new ColumnOptions {
					Id = Constants.ServiceTableColumnDescId,
					DisplayName = Constants.ServiceTableColumnDescId,
					DisplayOrder = 25,
					Width = 140,
					Type = ColumnType.Text
				},
				new ColumnOptions {
					Id = Constants.ServiceTableColumnPortId,
					DisplayName = Constants.ServiceTableColumnPortId,
					DisplayOrder = 30,
					Width = 60,
					Type = ColumnType.Text
				},
				new ColumnOptions {
					Id = Constants.ServiceTableColumnStatusId,
					DisplayName = Constants.ServiceTableColumnStatusId,
					DisplayOrder = 40,
					Width = 80,
					Type = ColumnType.Text
				},
				new ColumnOptions {
					Id = Constants.ServiceTableColumnLastHeartbeatId,
					DisplayName = Constants.ServiceTableColumnLastHeartbeatId,
					DisplayOrder = 50,
					Width = 160,
					Type = ColumnType.Text
				}
			};
		}

		/// <summary>
		/// Sets the group nodes details view.
		/// </summary>
		/// <param name="item">Item.</param>
		/// <param name="nodeType">Node type.</param>
		private void SetGroupNodesDetailsView (NSObject item, NodeType nodeType)
		{
			var c = new MainTableController { MainTableView = new NSTableView () };
			c.LoadView = () => {
				c.MainTableView.Delegate = new MainTableViewDelegate (c, this);

				var columnNames = GetHostColumnOptionsForMainTable();
				var siteName = nodeType == NodeType.Infrastructure
					? (item as InfrastucturesGroupNode).GetSiteName ()
					: (item as ManagementsGroupNode).GetSiteName ();
				var filteredHosts = _rootNode.FilterBy (nodeType, siteName);
				ReloadMainTable (c.MainTableView, columnNames, filteredHosts);
			};
			controller.DetailViewController = c;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Sets the group nodes details view.
		/// </summary>
		/// <param name="item">Item.</param>
		/// <param name="nodeType">Node type.</param>
		private void SetInfrastructureGroupNodesDetailsView (NSObject item, NodeType nodeType)
		{
			var c = new MainTableController { MainTableView = new NSTableView () };
			c.LoadView = () => {
				c.MainTableView.Delegate = new MainTableViewDelegate (c, this);
				var options = GetHostColumnOptionsForMainTable ();
				var columnOption1 = new ColumnOptions {
					Id = Constants.PscTableColumnLastPingId,
					DisplayName = Constants.PscTableColumnLastPingId,
					DisplayOrder = 40,
					Width = 160,
					Type = ColumnType.Text
				};
				options.Add (columnOption1);

				var columnOption2 = new ColumnOptions {
					Id = Constants.PscTableColumnLastResponseTimeId,
					DisplayName = Constants.PscTableColumnLastResponseTimeId,
					DisplayOrder = 41,
					Width = 160,
					Type = ColumnType.Text
				};
				options.Add (columnOption2);

				var columnOption3 = new ColumnOptions {
					Id = Constants.PscTableColumnLastErrorId,
					DisplayName = Constants.PscTableColumnLastErrorId,
					DisplayOrder = 42,
					Width = 160,
					Type = ColumnType.Text
				};
				options.Add (columnOption3);

				var siteName = nodeType == NodeType.Infrastructure 
					? (item as InfrastucturesGroupNode).GetSiteName ()
					: (item as ManagementsGroupNode).GetSiteName ();
				var filteredHosts = _rootNode.FilterBy (nodeType, siteName);
				ReloadMainTable (c.MainTableView, options, filteredHosts);
			};
			controller.DetailViewController = c;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Sets the site details view.
		/// </summary>
		/// <param name="item">Item.</param>
		private void SetSiteDetailsView (NSObject item)
		{
			var c = new MainTableController {MainTableView = new NSTableView() };
			c.LoadView = () => {
				c.MainTableView.Delegate = new MainTableViewDelegate (c,this);
				var columnNames = GetHostColumnOptionsForMainTable();
				var node = (item as SiteNode);
				var siteName = node.GetSiteName ();
				var filteredHosts = _rootNode.FilterBy (siteName);
				ReloadMainTable (c.MainTableView, columnNames, filteredHosts);
			};
			controller.DetailViewController = c;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Sets the global topology details view.
		/// </summary>
		/// <param name="item">Item.</param>
		private void SetGlobalTopologyDetailsView (NSObject item)
		{
			var c = new MainTableController {MainTableView = new NSTableView() };
			c.LoadView = () => {
				c.MainTableView.Delegate = new MainTableViewDelegate (c,this);
				var columnNames = GetGlobalViewColumnOptionsForMainTable();
				ReloadMainTable (c.MainTableView, columnNames, _rootNode.Hosts);
			};
			controller.DetailViewController = c;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Sets the infrastructure details view.
		/// </summary>
		/// <param name="item">Item.</param>
		private void SetInfrastructureDetailsView (string nodeName, string siteName, GlobalTopologyNode rootNode)
		{
			var infraController = new InfrastructureController (nodeName, rootNode) {
				SiteName = siteName
			};
			controller.DetailViewController = infraController;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Refreshs the table views based on selection.
		/// </summary>
		/// <param name="row">Row.</param>
		public void RefreshTableViewsBasedOnSelection (nint row)
		{
			UIErrorHelper.CheckedExec (delegate () {
				if (row >= (nint)0) {
					SetDefaultView ();
					NSObject item = controller.TreeView.ItemAtRow ((int)row);
					if (item is GlobalTopologyNode) {
						SetGlobalTopologyDetailsView (item);
					} else if (item is SiteNode) {
						SetSiteDetailsView (item);
					} else if (item is InfrastucturesGroupNode) {
						SetGroupNodesDetailsView (item, NodeType.Infrastructure);
					} else if (item is ManagementsGroupNode) {
						SetGroupNodesDetailsView (item, NodeType.Management);
					} else if (item is InfrastructureNode) {
						var infraNode = ((InfrastructureNode)item);
						var infraName = infraNode.DisplayName;
						var siteName = infraNode.Parent.Parent.DisplayName;
						var globalNode = infraNode.Parent.Parent.Parent as GlobalTopologyNode;
						SetInfrastructureDetailsView (infraName,siteName,globalNode);
					} else if (item is ManagementNode) {
						var mgmtNode = ((ManagementNode)item);
						var mgmtName = mgmtNode.DisplayName;
						var siteName = mgmtNode.Parent.Parent.DisplayName;
						var globalNode = mgmtNode.Parent.Parent.Parent as GlobalTopologyNode;
						SetMonitorView (mgmtName, siteName, globalNode);
					} else {
						SetDefaultView ();
					}
				}
			});
		}

		/// <summary>
		/// Sets the monitor view.
		/// </summary>
		private void SetMonitorView (string nodeName, string siteName, GlobalTopologyNode rootNode)
		{
			var serverDto = new ServerDto {
				Server = nodeName,
				Upn = _serverDto.Upn,
				UserName = _serverDto.UserName,
				Password = _serverDto.Password,
				DomainName = _serverDto.DomainName
			};
			var monitorController = new MonitorController (_service) {
				ServerDto = serverDto,
				RootNode = rootNode,
				SiteName = siteName
			};
			controller.DetailViewController = monitorController;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Sets the main table view.
		/// </summary>
		private void SetMainTableView()
		{
			var c = new MainTableController ();
			controller.DetailViewController = c;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Sets the default view.
		/// </summary>
		private void SetDefaultView()
		{
			var c = new DefaultEmptyController ();
			controller.DetailViewController = c;
			controller.RefreshDetailView ();
		}

		/// <summary>
		/// Call Async refresh.
		/// </summary>
		public async void AsyncRefresh()
		{
			var task = new Task(() => Refresh(false));
			task.Start ();
		}

		/// <summary>
		/// Call topology refresh.
		/// </summary>
		public void TopologyRefresh()
		{
			var task = new Task(() => Refresh(true));
			task.Start ();
		}

		public void PreRefresh()
		{
			ServerToolBarItem.Enabled = false;
			RefreshToolBarItem.Enabled = false;
			controller.SetRefreshText (Constants.LoadingTopologyDetails);
			controller.SetRefreshStatus (true);
		}

		/// <summary>
		/// Initialize this instance.
		/// </summary>
		private async void Refresh(bool topologyRefresh)
		{
			InvokeOnMainThread (PreRefresh);
			var success = true;
			try
			{
				if (_rootNode == null)
				{
					_rootNode = new GlobalTopologyNode (_service, _serverDto, _managementDto);
					_rootNode.OnCacheRefresh += OnCacheRefresh;
				}
				else
				{
					_rootNode.RefreshTopology (topologyRefresh);
					InvokeOnMainThread (_rootNode.UpdateNodes);
				}
			}
			catch(AggregateException exc) {
				InvokeOnMainThread (() => {
					controller.SetRefreshStatus (false);
					ServerToolBarItem.Enabled = true;
					RefreshToolBarItem.Enabled = true;
					success = false;
					if(exc.InnerExceptions.Count > 0)
					{
						var msg = exc.InnerExceptions.Select(x=>x.Message).Aggregate((x,y) => x + " , " + y);
						var message = string.Format(Constants.VMDirConnectFailure, msg);
						UIErrorHelper.ShowAlert (message, Constants.FailedLoadingTopologyDetails); 
					}
				});
			}
			catch(Exception exc) {
				
				InvokeOnMainThread (() => {
					controller.SetRefreshStatus (false);
					ServerToolBarItem.Enabled = true;
					RefreshToolBarItem.Enabled = true;
					success = false;
					UIErrorHelper.ShowAlert (exc.Message, Constants.FailedLoadingTopologyDetails); 
				});
			}

			if(success)
				InvokeOnMainThread (PostRefresh);
		}

		/// <summary>
		/// Raises the cache refresh event.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void OnCacheRefresh (object sender, EventArgs e)
		{
			if(_rootNode != null)
				InvokeOnMainThread (ReloadTableView);
		}

		/// <summary>
		/// Posts the refresh.
		/// </summary>
		private void PostRefresh()
		{
			try {
				controller.SetRefreshText (Constants.LoadingTopologyDetails);
				var treeDataSource = new OutlineViewDataSource (_rootNode);
				controller.TreeView.DataSource = treeDataSource;
				controller.TreeView.Activated += OnOutlineViewActivated;
				controller.TreeView.SelectionDidChange += OnOutlineViewActivated;
				controller.TreeView.OutlineTableColumn.HeaderCell.Title = "Connected to " + _serverDto.Server;
				controller.TreeView.Delegate = new OutlineDelegate (this);
				controller.TreeView.OutlineTableColumn.DataCell = new NSBrowserCell ();

				if ((int)controller.TreeView.SelectedRow <= 0) {
					controller.TreeView.SelectRow (0, true);
				}
				OnOutlineViewActivated (this, EventArgs.Empty);
			} catch (Exception exc) {
				// Log exception
			}
			controller.SetRefreshStatus (false);
			ServerToolBarItem.Enabled = true;
			RefreshToolBarItem.Enabled = true;
		}

		/// <summary>
		/// Raises the outline view activated event.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		public void OnOutlineViewActivated (object sender, EventArgs e)
		{
			nint row = controller.TreeView.SelectedRow;
			if (row >= (nint)0) {
				RefreshTableViewsBasedOnSelection (row);
			}
		}

		/// <summary>
		/// Sets the connect toolbar based on state.
		/// </summary>
		/// <param name="state">State.</param>
		public void SetConnectToolbar(ServerState state)
		{
			if (state == ServerState.Disconnected) {
				ServerToolBarItem.Image = NSImage.ImageNamed (Constants.ConnectIcon);
				ServerToolBarItem.Label = Constants.ConnectLabel;
				RefreshToolBarItem.Active = false;
				ServerToolBarItem.Active = true;
			} else {
				ServerToolBarItem.Image = NSImage.ImageNamed (Constants.DisconnectIcon);
				ServerToolBarItem.Label = Constants.DisconnectLabel;
				RefreshToolBarItem.Active = true;
				ServerToolBarItem.Active = true;
			}
			_serverState = state;
		}

		/// <summary>
		/// Raises the server tool bar item activated event.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void OnServerToolBarItem_Activated (object sender, EventArgs e)
		{
			if(_serverState == ServerState.Connected)
			{
				_serverState = ServerState.Disconnected;
				_isLoggedIn = false;
				if (_rootNode != null) {
					_rootNode.OnCacheRefresh -= OnCacheRefresh;
				}
				_rootNode = null;
				_managementDto = null;
				ServerToolBarItem.Image = NSImage.ImageNamed (Constants.ConnectIcon);
				ServerToolBarItem.Label = Constants.ConnectLabel;
				_serverDto = null;
				RefreshToolBarItem.Active = false;
				for(int index = 0; index < this.ContainerView.Subviews.Length; index++)
				{
					this.ContainerView.Subviews[index].RemoveFromSuperview();
				}
				LoggedInLabel.StringValue = (NSString)string.Empty;
			}
			else
			{
				ConnectToServer();
			}
		}

		/// <summary>
		/// Pings the host to check if it is reachable.
		/// </summary>
		/// <returns><c>true</c>, if host was pinged, <c>false</c> otherwise.</returns>
		/// <param name="nameOrAddress">Name or address.</param>
		public static bool PingHost(string nameOrAddress)
		{
			bool pingable = false;
			Ping pinger = new Ping();
			try
			{
				PingReply reply = pinger.Send(nameOrAddress);
				pingable = reply.Status == IPStatus.Success;
			}
			catch (Exception)
			{
				// Discard PingExceptions and return false;
			}
			return pingable;
		}

		/// <summary>
		/// Logins to server.
		/// </summary>
		/// <returns>The to server.</returns>
		private void LoginToServer()
		{
			_managementDto = _service.Connect (_serverDto);
			_isLoggedIn = _managementDto != null;
		}
	}

	/// <summary>
	/// Tracks the state of the server.
	/// </summary>
	public enum ServerState
	{
		Connected,
		Disconnected
	}
}
