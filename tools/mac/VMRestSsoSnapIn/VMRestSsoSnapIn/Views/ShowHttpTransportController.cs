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

using AppKit;
using Foundation;
using System;
using System.Linq;
using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class ShowHttpTransportController : NSWindowController
	{
		public ShowHttpTransportController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public ShowHttpTransportController (NSCoder coder) : base (coder)
		{
		}

		public ShowHttpTransportController () : base ("ShowHttpTransport")
		{
			
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			var httpTransportCollection =  SnapInContext.Instance.ServiceGateway.HttpTransport.GetAll ();
			httpTransportCollection.Sort (SortingOrder.Descending);
			var items = httpTransportCollection.Items.Select(x=> new HttpTransportViewData { Name = x.RequestUri, Method = x.Method }).ToList();
			ReloadTableView(HttpTransportTableView, items, httpTransportCollection);

			if (items.Count > 0)
				HttpTransportTableView.SelectRow (0, false);
		}

		public new ShowHttpTransport Window {
			get { return (ShowHttpTransport)base.Window; }
		}
		public void ReloadTableView(NSTableView tableView, List<HttpTransportViewData> datasource, HttpTransportCollection collection)
		{
			foreach(NSTableColumn column in tableView.TableColumns())
			{
				tableView.RemoveColumn (column);
			}
			tableView.Delegate = new TableDelegate (this);
			var listView = new HttpTransportDataSource { Entries = datasource, HttpTransportCollection = collection };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Method", DisplayName = "", DisplayOrder = 1, Width = 60, Type = ColumnType.Text },
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 2, Width = 400, Type = ColumnType.Text }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				tableView.AddColumn (column);
			}
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}

		public class TableDelegate : NSTableViewDelegate
		{
			private ShowHttpTransportController _controller;
			public TableDelegate (ShowHttpTransportController controller)
			{
				_controller = controller;
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					NSTextFieldCell textCell = cell as NSTextFieldCell;
					if (textCell != null) {
						var collection = ((HttpTransportDataSource)(_controller.HttpTransportTableView.DataSource)).HttpTransportCollection;
						if (collection != null) {
							var item = collection [(int)row];
							if (tableColumn.Identifier == "Method") {
								if (item.Method == "GET") {
									textCell.TextColor = NSColor.Magenta;
										
								} else if (item.Method == "POST") {
									textCell.TextColor = NSColor.Brown;

								} else if (item.Method == "PUT") {
									textCell.TextColor = NSColor.Blue;

								} else if (item.Method == "DELETE") {
									textCell.TextColor = NSColor.Orange;
								}
							} else {
								if (tableColumn.Identifier == "Name" && !string.IsNullOrEmpty (item.Error)) {
									textCell.TextColor = NSColor.Red;
								}
								else
									textCell.TextColor = NSColor.Black;
							}
						}
					}
				});
			}

			public override void SelectionDidChange (NSNotification notification)
			{
				ActionHelper.Execute (delegate {
					var row = (int)_controller.HttpTransportTableView.SelectedRow;

					var collection = ((HttpTransportDataSource)(_controller.HttpTransportTableView.DataSource)).HttpTransportCollection;
						if (row >= 0 && collection != null) {
							var item = collection[row];
						_controller.TxtServerName.StringValue = string.IsNullOrEmpty (item.Server) ? string.Empty : item.Server;
						_controller.TxtRequestUri.StringValue = string.IsNullOrEmpty (item.RequestUri) ? string.Empty : item.RequestUri;
						_controller.TxtTimestamp.StringValue = item.RequestTimestamp.ToString("MM/dd/yyyy hh:mm:ss");
						_controller.TxtTimeTaken.StringValue = string.IsNullOrEmpty (item.TimeTaken) ? string.Empty : item.TimeTaken;
						_controller.TxtHttpMethod.StringValue = string.IsNullOrEmpty (item.Method) ? string.Empty : item.Method;

						if (item.RequestHeader != null) {
							_controller.TxtContentType.StringValue = string.IsNullOrEmpty (item.RequestHeader.ContentType) ? string.Empty : item.RequestHeader.ContentType;
							_controller.TxtContentType.StringValue = string.IsNullOrEmpty (item.RequestHeader.ContentLength) ? string.Empty : item.RequestHeader.ContentLength;
						}
						_controller.TxtRequestData.Value = ((NSString)(string.IsNullOrEmpty (item.RequestData) ? string.Empty : item.RequestData));

						if (item.ResponseHeader != null) {
							_controller.TxtReponseContentType.StringValue = string.IsNullOrEmpty (item.ResponseHeader.ContentType) ? string.Empty : item.ResponseHeader.ContentType;
							_controller.TxtReponseContentType.StringValue = string.IsNullOrEmpty (item.ResponseHeader.ContentLength) ? string.Empty : item.ResponseHeader.ContentLength;
						}
						_controller.TxtResponseData.Value = ((NSString)(string.IsNullOrEmpty (item.ResponseData) ? string.Empty : item.ResponseData));
						_controller.TxtError.Value = ((NSString)(string.IsNullOrEmpty (item.Error) ? string.Empty : item.Error));
					}
				});
			}
		}
	}
}
