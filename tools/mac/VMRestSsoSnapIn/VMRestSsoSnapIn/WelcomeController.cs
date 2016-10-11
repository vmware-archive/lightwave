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
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace RestSsoAdminSnapIn
{
	public partial class WelcomeController : NSWindowController
	{
		public WelcomeController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public WelcomeController (NSCoder coder) : base (coder)
		{
		}

		public WelcomeController () : base ("Welcome")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			//set window background color
			this.Window.BackgroundColor = NSColor.FromSrgb (1, 1, (float)1, (float)1);
			ConnectPopupButton.AddItem ("New Server");
			var tokens = SnapInContext.Instance.AuthTokenManager.GetAllAuthTokens ();
			var servers = tokens.Where(x=>x.ServerDto != null).OrderByDescending(x=>x.ServerDto.UpdatedOn).Select(x=> x.ServerDto.ServerName).Take(3).ToArray();
			ConnectPopupButton.AddItems(servers);
		}

		partial void OnConnect (Foundation.NSObject sender)
		{
			ActionHelper.Execute(delegate() {
					
				if (ConnectPopupButton.SelectedItem.Title != "New Server")
				{	
					var server = ConnectPopupButton.SelectedItem.Title;
					var tokens = SnapInContext.Instance.AuthTokenManager.GetAllAuthTokens ();
					var serverDto = tokens.Where(x=>x.ServerDto != null && x.ServerDto.ServerName == server).Select(x=>x.ServerDto).FirstOrDefault();
					if (!WebUtil.PingHost (serverDto.ServerName)) {
						UIErrorHelper.ShowAlert ("Server name or ip address no longer exists or not reachable", "Alert");
						return;
					}
					else
					{
						var mainWindowController = new MainWindowController (serverDto);
						mainWindowController.Window.MakeKeyAndOrderFront (null);
					}
				}
				else
				{
					var mainWindowController = new MainWindowController ();
					mainWindowController.Window.MakeKeyAndOrderFront (null);
				}
				this.Window.IsVisible = false;
			});
		}
		public new Welcome Window {
			get { return (Welcome)base.Window; }
		}
	}
}
