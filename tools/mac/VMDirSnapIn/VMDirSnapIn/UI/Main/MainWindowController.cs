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
using AppKit;
using Foundation;
using Nodes;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDirSnapIn.DataSource;
using VMDirSnapIn.Delegate;
using VMDirSnapIn.Nodes;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using System.Collections.Generic;
using System.Linq;

namespace VMDirSnapIn.UI
{
	public partial class MainWindowController : NSWindowController
	{
		private VmdirSplitViewController splitViewController;
		private OutlineViewDataSource outlineViewDataSource;
		private OutlineViewNavigationController navigationController;
		private NSTableView MainTableView;
		public NSOutlineView MainOutlineView;
		private VMDirServerDTO serverNode;

		//observers
		private NSObject ReloadOutlineViewNotificationObject;
		private NSObject ReloadTableViewNotificationObject;
		private NSObject CloseNotificationObject;

		private List<VMDirServerDTO> server { get; set; }

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
		public MainWindowController(List<VMDirServerDTO> serverName)
			: base("MainWindow")
		{
			Initialise();
			server = serverName;
		}

		private void Initialise()
		{
			serverNode = VMDirServerDTO.CreateInstance();
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
				splitViewController = new VmdirSplitViewController();
				this.ContainerView.AddSubview(splitViewController.View);

				SetToolBarState(false);
				(NSApplication.SharedApplication.Delegate as AppDelegate).OpenConnectionMenuITem.Hidden = true;

				//Notifications for OutlineView and Tableview to reload
				ReloadOutlineViewNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadOutlineView", ReloadOutlineView);
				ReloadTableViewNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadTableView", ReloadTableView);
				CloseNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"CloseApplication", OnCloseConnectionNotificationReceived);
				PageSizeToolBarItem.Active = true;
			}
			catch (Exception e)
			{
				System.Diagnostics.Debug.WriteLine("Error : " + e.Message);
				UIErrorHelper.ShowAlert("", e.Message);
			}
		}

		private void InitialiseViews()
		{
			AppDelegate appDelegate = NSApplication.SharedApplication.Delegate as AppDelegate;
			appDelegate.OpenConnectionMenuITem.Hidden = true;
			try
			{
				if (serverNode.IsLoggedIn)
				{
					InitialiseDefaultOutlineView();
					var indx = server.FindIndex(x => string.Equals(x.Server, serverNode.Server));
					if (indx >= 0)
						server.RemoveAt(indx);
					VMDirSnapInEnvironment.Instance.LocalData.AddServer(serverNode);
					DirectoryNode baseNode = new DirectoryNode(serverNode.BaseDN, new List<string>() { string.Empty }, serverNode, null);
					baseNode.IsBaseNode = true;
					outlineViewDataSource = new OutlineViewDataSource(baseNode);
					splitViewController.VmdirOutlineView.DataSource = outlineViewDataSource;
					baseNode.Expand(serverNode.BaseDN);
					SetToolBarState(true);
					InitialiseDefaultTableView();
					StatusLabel.StringValue = "Logged in : " + serverNode.BindDN;
				}
				else
					UIErrorHelper.ShowAlert(VMDirConstants.ERR_LOGIN_FAILED, "Login not successful!");
			}
			catch (Exception e)
			{
				CloseConnection();
				UIErrorHelper.ShowAlert(e.Message, "Login not successful!");
			}
		}

		private void InitialiseDefaultOutlineView()
		{
			MainOutlineView = splitViewController.VmdirOutlineView;
			MainOutlineView.OutlineTableColumn.HeaderCell.Title = " Connected to " + serverNode.Server;

			MainOutlineView.Activated += OnOutlineViewActivated;

			var col = MainOutlineView.OutlineTableColumn;
			if (col != null)
				col.DataCell = new NSBrowserCell();
			MainOutlineView.Delegate = new OutlineDelegate(this);
		}

		private void InitialiseDefaultTableView()
		{
			MainTableView = splitViewController.propViewController.PropTableView;
			RemoveTableColumns();

			//Populate appropriate columns
			NSTableColumn col = new NSTableColumn("Attribute");
			col.HeaderCell.Title = "Attribute";
			col.HeaderCell.Alignment = NSTextAlignment.Center;
			col.DataCell = new NSBrowserCell();
			col.MinWidth = 250;
			col.ResizingMask = NSTableColumnResizing.UserResizingMask;
			MainTableView.AddColumn(col);

			NSTableColumn col1 = new NSTableColumn("Value");
			col1.HeaderCell.Title = "Value";
			col1.ResizingMask = NSTableColumnResizing.UserResizingMask;
			col1.HeaderCell.Alignment = NSTextAlignment.Center;
			col1.MinWidth = 250;
			MainTableView.AddColumn(col1);

			NSTableColumn col2 = new NSTableColumn("Syntax");
			col2.HeaderCell.Title = "Syntax";
			col2.ResizingMask = NSTableColumnResizing.UserResizingMask;
			col2.HeaderCell.Alignment = NSTextAlignment.Center;
			col2.MinWidth = 200;
			MainTableView.AddColumn(col2);
		}

		public async void ConnectToServer(List<VMDirServerDTO> server)
		{
			ProgressWindowController pwc = new ProgressWindowController();
			IntPtr session = new IntPtr(0);
			ConnectToLdapWindowController awc = new ConnectToLdapWindowController(server);
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
					serverNode = awc.ServerDTO;
					await serverNode.DoLogin();
					InitialiseViews();
				}
			}
			catch (Exception e)
			{
				serverNode.IsLoggedIn = false;
				UIErrorHelper.ShowAlert(VMDirConstants.ERR_LOGIN_FAILED + " : " + e.Message, "Login not successful!");
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
			DeleteObjectToolBarItem.Active = state;
			AddUserToolBarItem.Active = state;
			AddGroupToolBarItem.Active = state;
			BackForwardToolBarItem.Active = state;
			RefreshToolBarItem.Active = state;
			SuperLogToolBarItem.Active = state;
			OperationalToolBarItem.Active = state;
			SearchToolBarItem.Active = state;
			FetchNextPageToolBarItem.Active = state;
			OptionalToolBarItem.Active = state;
		}

		partial void ShowSuperLogWindow(NSObject sender)
		{
			SuperLoggingBrowserWindowController awc = new SuperLoggingBrowserWindowController(serverNode);
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
				ConnectToServer(server);
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

		private bool isObjectSelected(nint row)
		{
			if (row < (nint)0)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
				return false;
			}
			else
				return true;
		}

		partial void AddObject(NSObject sender)
		{
			nint row = MainOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
				node.ShowAddWindow();
			}
		}

		partial void AddUser(NSObject sender)
		{
			nint row = MainOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
				node.ShowAddUser();
			}
		}

		partial void AddGroup(NSObject sender)
		{
			nint row = MainOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
				node.ShowAddGroup();
			}
		}

		partial void DeleteObject(NSObject sender)
		{
			nint row = MainOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
				node.PerformDelete();
			}
		}

		public void CloseConnection()
		{
			UIErrorHelper.CheckedExec(delegate ()
				{
					serverNode.Connection.CloseConnection();
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

		public void RefreshTableViewBasedOnSelection(nint row)
		{
			UIErrorHelper.CheckedExec(delegate
			{
				if (row >= (nint)0)
				{
					NSObject item = MainOutlineView.ItemAtRow(row);
					if (item is DirectoryNode)
					{
						DirectoryNode node = item as DirectoryNode;
						MainTableView.DataSource = new PropertiesTableViewDataSource(node.Dn, node.ObjectClass[node.ObjectClass.Count - 1], node.ServerDTO, node.NodeProperties);
						splitViewController.propViewController.ds = (PropertiesTableViewDataSource)MainTableView.DataSource;
						MainTableView.Delegate = new PropertiesTableDelegate(this, (PropertiesTableViewDataSource)MainTableView.DataSource, splitViewController.propViewController);
					}
				}
				else
				{
					MainTableView.DataSource = null;
				}
				MainTableView.ReloadData();
			});
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

		partial void OnOperationalToolBarItem(NSObject sender)
		{
			if (serverNode.OperationalAttrFlag)
				serverNode.OperationalAttrFlag = false;
			else
				serverNode.OperationalAttrFlag = true;
			RefreshTableViewBasedOnSelection(MainOutlineView.SelectedRow);
		}

		partial void OnOptionalToolBatItem(NSObject sender)
		{
			if (serverNode.OptionalAttrFlag)
				serverNode.OptionalAttrFlag = false;
			else
				serverNode.OptionalAttrFlag = true;
			RefreshTableViewBasedOnSelection(MainOutlineView.SelectedRow);
		}

		partial void OnPageSizeToolBarItem(NSObject sender)
		{
			PageSizeController pswc = new PageSizeController(serverNode.PageSize);
			NSApplication.SharedApplication.BeginSheet(pswc.Window, this.Window, () =>
				{
				});
			try
			{
				nint result = NSApplication.SharedApplication.RunModalForWindow(pswc.Window);
				if (result == (nint)VMIdentityConstants.DIALOGOK)
				{
					serverNode.PageSize = pswc.PageSize;
				}
			}
			finally
			{
				Window.EndSheet(pswc.Window);
				pswc.Dispose();
			}
		}

		partial void OnSearchToolBarItem(NSObject sender)
		{
			nint row = MainOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
				node.ShowSearch();
			}
		}

		partial void OnFetchNextPageToolBarItem(NSObject sender)
		{
			nint row = MainOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNode node = MainOutlineView.ItemAtRow(row) as DirectoryNode;
				node.GetNextPage();
			}
		}
	}
}
