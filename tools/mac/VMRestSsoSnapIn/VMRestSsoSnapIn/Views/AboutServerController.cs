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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace RestSsoAdminSnapIn
{
	public partial class AboutServerController : NSWindowController
	{
		private ServerInfoDto _serverInfoDto;

		public AboutServerController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AboutServerController (NSCoder coder) : base (coder)
		{
		}

		public AboutServerController () : base ("AboutServer")
		{
		}

		public AboutServerController (ServerInfoDto serverInfoDto) : base ("AboutServer")
		{
			_serverInfoDto = serverInfoDto;
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			if (_serverInfoDto != null) {
				ProductTextField.StringValue = _serverInfoDto.ProductName == null ? string.Empty : _serverInfoDto.ProductName;
				ReleaseTextField.StringValue = _serverInfoDto.Release == null ? string.Empty : _serverInfoDto.Release;
				VersionTextField.StringValue = _serverInfoDto.Version == null ? string.Empty : _serverInfoDto.Version;
			}
		}

		public new AboutServer Window {
			get { return (AboutServer)base.Window; }
		}
	}
}
