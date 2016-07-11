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
using CoreGraphics;
using Foundation;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDir.Common.VMDirUtilities;
using VMDirSnapIn.DataSource;
using VMDirSnapIn.UI;

namespace VMDirSnapIn.Delegate
{
	public class PropertiesTableDelegate : NSTableViewDelegate
	{
		private PropertiesTableViewDataSource ds;
		private NSWindowController Controller;

		public PropertiesTableDelegate(NSWindowController controller, PropertiesTableViewDataSource ds)
		{
			this.Controller = controller;
			this.ds = ds;
		}

		public override nint GetNextTypeSelectMatch(NSTableView tableView, nint startRow, nint endRow, string searchString)
		{
			nint row = 0;
			foreach (var item in ds.displayAttrDTOList)
			{
				if (item.Name.Contains(searchString)) return row;
				++row;
			}
			return 0;
		}

		private void ConfigureTextField(NSTableCellView view, nint row)
		{
			// Add to view
			view.TextField.AutoresizingMask = NSViewResizingMask.WidthSizable;
			view.AddSubview(view.TextField);

			// Configure
			view.TextField.BackgroundColor = NSColor.Clear;
			view.TextField.Bordered = false;
			view.TextField.Selectable = false;
			if (string.Equals(view.Identifier, "Value"))
				view.TextField.Editable = true;
			else
				view.TextField.Editable = false;

			// Wireup events
			view.TextField.EditingEnded += (sender, e) =>
				{

					// Take action based on type
					switch (view.Identifier)
					{
						case "Value":
							string currKey = ds.displayAttrDTOList[(int)view.TextField.Tag].Name;

							if (currKey != "objectClass")
							{
								ds.displayAttrDTOList[(int)view.TextField.Tag].Value = view.TextField.StringValue;
								ds.modData.Add(currKey);
							}
							break;
					}
				};

			// Tag view
			view.TextField.Tag = row;
		}

		public override NSView GetViewForItem(NSTableView tableView, NSTableColumn tableColumn, nint row)
		{
			NSTableCellView view = (NSTableCellView)tableView.MakeView(tableColumn.Title, this);
			view = new NSTableCellView();

			// Configure the view
			view.Identifier = tableColumn.Title;

			// Take action based on title
			switch (tableColumn.Title)
			{
				case "Attribute":
					view.TextField = new NSTextField(new CGRect(0, 0, 250, 17));
					ConfigureTextField(view, row);
					break;
				case "Value":
					view.TextField = new NSTextField(new CGRect(0, 0, 250, 17));
					ConfigureTextField(view, row);
					break;
				case "Syntax":
					view.TextField = new NSTextField(new CGRect(16, 0, 200, 17));
					ConfigureTextField(view, row);
					var button = new NSButton(new CGRect(0, 0, 16, 16));
					button.SetButtonType(NSButtonType.MomentaryLightButton);
					button.Image = new NSImage("Question.png");
					button.Title = "";
					button.Tag = row;

					// Wireup events
					button.Activated += (sender, e) =>
					{
						// Get button and product
						var btn = sender as NSButton;
						var name = ds.displayAttrDTOList[(int)btn.Tag].Name;
						var type = ds.serverDTO.Connection.SchemaManager.GetAttributeType(name);
						AttributeHelpDTO attrHelp = null;
						if (type.AttributeSyntax != null)
							VMDirCommonEnvironment.Instance.AttrHelpDict.TryGetValue(type.AttributeSyntax, out attrHelp);

						SyntaxHelpWindowController shwc = new SyntaxHelpWindowController(attrHelp);
						NSApplication.SharedApplication.BeginSheet(shwc.Window, Controller.Window, () =>
							{
							});
						try
						{
							NSApplication.SharedApplication.RunModalForWindow(shwc.Window);
						}
						finally
						{
							Controller.Window.EndSheet(shwc.Window);
							shwc.Dispose();
						}
					};
					view.AddSubview(button);

					break;
			}

			switch (tableColumn.Title)
			{
				case "Attribute":
					view.TextField.StringValue = ds.displayAttrDTOList[(int)row].Name;
					view.TextField.Tag = row;
					break;
				case "Value":
					view.TextField.StringValue = ds.displayAttrDTOList[(int)row].Value == null ? string.Empty : ds.displayAttrDTOList[(int)row].Value;
					view.TextField.Tag = row;
					break;
				case "Syntax":
					view.TextField.StringValue = ds.displayAttrDTOList[(int)row].AttrSyntaxDTO.Type == null ? string.Empty : ds.displayAttrDTOList[(int)row].AttrSyntaxDTO.Type;
					foreach (NSView subview in view.Subviews)
					{
						var bt = subview as NSButton;
						if (bt != null)
						{
							bt.Tag = row;
						}
					}
					break;
			}
			return view;
		}

		/*public override bool ShouldSelectRow(NSTableView tableView, nint row)
		{
			return false;
		}*/
	}
}

