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

using Microsoft.ManagementConsole;
using System;
using System.IO;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;

namespace Vmware.Tools.RestSsoAdminSnapIn.Helpers
{
    public static class ActionHelper
    {
        public static void Execute(System.Action fn, AuthTokenDto authTokenDto)
        {
            Execute(fn, authTokenDto, null);
        }
        public static void Execute(System.Action fn, AuthTokenDto authTokenDto, string tenantName)
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
                                    // Refresh token
                                    if (authTokenDto.Refresh())
                                    {
                                        SnapInContext.Instance.AuthTokenManager.SetAuthToken(authTokenDto);
                                        fn();
                                    }
                                }, authTokenDto);
                            }
                            else
                            {
                                ErrorMessageDisplayHelper.ShowException(new Exception(error.Description));
                            }
                        }
                    }
                    else
                    {
                        if (response != null && response.StatusCode == HttpStatusCode.BadRequest && response.ContentType == "application/json;charset=UTF-8")
                        {
                            var resp = new StreamReader(response.GetResponseStream()).ReadToEnd();
                            var error = JsonConvert.Deserialize<AuthErrorDto>(resp);
                            if (resp.Contains(AuthError.InvalidGrant))
                            {                                
                                if (error != null)
                                {
                                    if (error.Error == AuthError.InvalidGrant && authTokenDto!= null && authTokenDto.Login != null)
                                    {
                                        // Session expired
                                        var sessionExpired = true;
                                        var root = SnapInContext.Instance.SnapIn.RootNode as SnapInNode;
                                        ServerNode node = null;
                                        foreach (ServerNode child in root.Children)
                                        {
                                            if (child.DisplayName == authTokenDto.ServerDto.ServerName)
                                                node = child;
                                        }
                                        var serverNode = root.GetServerNode(node);
                                        if (serverNode != null)
                                            serverNode.Login(sessionExpired, tenantName);
                                    }
                                    else
                                    {
                                        ErrorMessageDisplayHelper.ShowException(new Exception(error.Description));
                                    }
                                }
                                else
                                {
                                    ErrorMessageDisplayHelper.ShowException(new Exception(error.Description));
                                }
                            }
                            else
                            {
                                ErrorMessageDisplayHelper.ShowException(new Exception(error.Description));
                            }
                        }
                        else
                        {
                            ErrorMessageDisplayHelper.ShowException(exp);
                        }
                    }
                }
                else
                {
                    ErrorMessageDisplayHelper.ShowException(exp);
                }
            }
            catch (Exception exp)
            {
                ErrorMessageDisplayHelper.ShowException(exp);
            }
        }
    }
}