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
using System.IO;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using RestSsoAdminSnapIn;


namespace Vmware.Tools.RestSsoAdminSnapIn.Helpers
{
	public static class ActionHelper
	{
		public static void Execute(System.Action fn)
		{
			try
			{
				fn();
			}
			catch (WebException exp)
			{
				if (exp.Response is HttpWebResponse)
				{
					var response = exp.Response as HttpWebResponse;
					if (response != null && response.StatusCode == HttpStatusCode.Unauthorized)
					{
						var resp = new StreamReader(exp.Response.GetResponseStream()).ReadToEnd();
						var error = JsonConvert.Deserialize<AuthErrorDto>(resp);
						if (error != null)
						{
							if (error.Error == AuthError.InvalidToken)
							{
								ActionHelper.Execute(delegate()
									{
										NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshToken", new NSObject());
									});
							}
							else
							{
								UIErrorHelper.ShowAlert(error.Details, "Error");
							}
						}
					}
					else
					{
						if (response != null && response.StatusCode == HttpStatusCode.BadRequest && response.ContentType == "application/json;charset=UTF-8") {
							var resp = new StreamReader (response.GetResponseStream ()).ReadToEnd ();
							var error = JsonConvert.Deserialize<AuthErrorDto> (resp);
							if (resp.Contains (AuthError.InvalidGrant)) {                               
								if (error != null) {                                  
									if (error.Error == AuthError.InvalidGrant) {
										NSNotificationCenter.DefaultCenter.PostNotificationName ("LoggedInSessionExpired", new NSObject ());
									} else {
										UIErrorHelper.ShowAlert (error.Details, "Error");
									}
								} else {
									UIErrorHelper.ShowAlert (error.Details, "Error");
								}
							} else {
								UIErrorHelper.ShowAlert (error.Details, "Error");
							}
						} else if (response != null && response.ContentType == "application/json") {
							var resp = new StreamReader (response.GetResponseStream ()).ReadToEnd ();
							UIErrorHelper.ShowAlert (exp.Message + " Details: " + resp, "Error");
						}
						else
						{
							UIErrorHelper.ShowAlert(exp.Message, "Error");
						}
					}
				}
				else
				{
					UIErrorHelper.ShowAlert(exp.Message, "Error");
				}
			}
			catch (Exception exp)
			{
				UIErrorHelper.ShowAlert(exp.Message, "Error");
			}
		}
	}
}

