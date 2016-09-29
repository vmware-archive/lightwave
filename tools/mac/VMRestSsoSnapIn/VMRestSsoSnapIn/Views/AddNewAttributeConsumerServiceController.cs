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
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;


namespace RestSsoAdminSnapIn
{
	public partial class AddNewAttributeConsumerServiceController : NSWindowController
	{
		private List<AttributeDto> _attributes;
		public bool DefaultSet { get; set; }
		public AttributeConsumerServiceDto AttributeConsumerServiceDto { get; set;}
		public bool IsUpdated { get; private set;}

		public AddNewAttributeConsumerServiceController (IntPtr handle) : base (handle)
		{
			Initialize();
		}

		[Export ("initWithCoder:")]
		public AddNewAttributeConsumerServiceController (NSCoder coder) : base (coder)
		{
			Initialize();
		}

		public AddNewAttributeConsumerServiceController () : base ("AddNewAttributeConsumerService")
		{
			Initialize();
		}

		private void Initialize()
		{
			_attributes = new List<AttributeDto> ();
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			IsUpdated = false;
			//Events
			this.BtnAdd.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Name has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtIndex.StringValue))
				{
					UIErrorHelper.ShowAlert ("Index has invalid value", "Alert");
				} else if(ChDefault.StringValue == "1" && DefaultSet && (AttributeConsumerServiceDto != null && !AttributeConsumerServiceDto.IsDefault))
				{
					UIErrorHelper.ShowAlert ("Multiple attribute consumer services chosen as default", "Alert");
				}
				else
				{
					AttributeConsumerServiceDto = new AttributeConsumerServiceDto
					{
						Name = TxtName.StringValue,
						Index = TxtIndex.IntValue,
						IsDefault = ChDefault.StringValue == "1",
						Attributes = (AttributeTableView.DataSource as AttributeDataSource).Entries
					};
					IsUpdated = true;
					this.Close ();
					NSApplication.SharedApplication.StopModalWithCode (0);
				}
			};
			this.BtnAddAttribute.Activated += (object sender, EventArgs e) => {

				if(string.IsNullOrEmpty(TxtAttributeName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Attribute name cannot be empty", "Alert");
					return;
				}
				if(string.IsNullOrEmpty(TxtFriendlyName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Attribute friendly name cannot be empty", "Alert");
					return;
				}
				if(string.IsNullOrEmpty(TxtNameFormat.StringValue))
				{
					UIErrorHelper.ShowAlert ("Attribute name format cannot be empty", "Alert");
					return;
				}
				var attributeDto = new AttributeDto
				{
					Name = TxtAttributeName.StringValue,
					FriendlyName = TxtFriendlyName.StringValue,
					NameFormat = TxtNameFormat.StringValue
				};
				TxtAttributeName.StringValue = string.Empty;
				TxtFriendlyName.StringValue = string.Empty;
				TxtNameFormat.StringValue = string.Empty;
				_attributes.Add(attributeDto);
				AttributeTableView.DataSource = new AttributeDataSource { Entries = _attributes };
				AttributeTableView.ReloadData ();
			};
			this.BtnRemoveAttribute.Activated += (object sender, EventArgs e) => {

				if(AttributeTableView.SelectedRow > -1)
				{
					_attributes.RemoveAt((int)AttributeTableView.SelectedRow);
					AttributeTableView.DataSource = new AttributeDataSource { Entries = _attributes };
					AttributeTableView.ReloadData ();
				}
			};
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			if (AttributeConsumerServiceDto != null) {
				TxtName.StringValue = AttributeConsumerServiceDto.Name;
				_attributes = AttributeConsumerServiceDto.Attributes;
				ChDefault.StringValue = AttributeConsumerServiceDto.IsDefault ? "1" : "0";
				TxtIndex.IntValue = AttributeConsumerServiceDto.Index;
			}
			InitializeAttributes ();
		}

		public new AddNewAttributeConsumerService Window {
			get { return (AddNewAttributeConsumerService)base.Window; }
		}
		private void InitializeAttributes()
		{
			AttributeTableView.Delegate = new TableDelegate (this);
			AttributeTableView.DataSource = new AttributeDataSource { Entries = _attributes };
			AttributeTableView.ReloadData ();
		}
		public class TableDelegate : NSTableViewDelegate
		{
			private AddNewAttributeConsumerServiceController _controller;
			public TableDelegate (AddNewAttributeConsumerServiceController controller)
			{
				_controller = controller;
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						browserCell.Editable = true;
					}
				});
			}

			public override void SelectionDidChange (NSNotification notification)
			{
				var source = _controller.AttributeTableView.DataSource as AttributeDataSource;
				var row = (int)_controller.AttributeTableView.SelectedRow;
				if (row > -1 && row < source.Entries.Count) {
					if (!string.IsNullOrEmpty (source.Entries [row].Name))
						_controller.TxtAttributeName.StringValue = source.Entries [row].Name;
					if (!string.IsNullOrEmpty (source.Entries [row].FriendlyName))
						_controller.TxtFriendlyName.StringValue = source.Entries [row].FriendlyName;
					if (!string.IsNullOrEmpty (source.Entries [row].NameFormat))
						_controller.TxtNameFormat.StringValue = source.Entries [row].NameFormat;
				}
			}
		}
	}
}
