/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Text;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.HttpTransport
{
    public class HttpTransportService
    {
        private HttpTransportCollection cachedCollection;

        public HttpTransportService()
        {
            cachedCollection = new HttpTransportCollection(5000);
        }

        public void HandleMessage(WebMessageTrace messageTrace)
        {
            var httpTransportDto = new HttpTransportItem { Id = messageTrace.Id, Server = messageTrace.Server };

            var message = messageTrace.Messages.FirstOrDefault(x => x.Type == WebMessageType.Request);
            {
                httpTransportDto.RequestData = MaskPassword(message);
                httpTransportDto.RequestHeader = new HttpTransportRequestHeader(message.Headers);
                httpTransportDto.RequestTimestamp = message.Timestamp;
                httpTransportDto.Method = message.Method;
                httpTransportDto.RequestUri = message.AbsolutePath;
            }

            message = messageTrace.Messages.FirstOrDefault(x => x.Type == WebMessageType.Response);
            if (message != null)
            {
                httpTransportDto.ResponseData = message.Details;
                httpTransportDto.ResponseHeader = new HttpTransportResponseHeader(message.Headers);
                TimeSpan span = message.Timestamp.Subtract(httpTransportDto.RequestTimestamp);
                httpTransportDto.TimeTaken = string.Format("{0}:{1}:{2}.{3}", span.Hours, span.Minutes, span.Seconds, span.Milliseconds);
            }

            message = messageTrace.Messages.FirstOrDefault(x => x.Type == WebMessageType.Error);
            if (message != null)
            {
                httpTransportDto.Error = message.Details;
                httpTransportDto.ResponseHeader = new HttpTransportResponseHeader(message.Headers);
                TimeSpan span = message.Timestamp.Subtract(httpTransportDto.RequestTimestamp);
                httpTransportDto.TimeTaken = string.Format("{0}:{1}:{2}.{3}", span.Hours, span.Minutes, span.Seconds, span.Milliseconds);
            }
            cachedCollection.Add(httpTransportDto);
        }
        private string MaskPassword(WebMessage message)
        {
            // OAuth login
            if (message.AbsolutePath.StartsWith(ServiceConfigManager.LoginTokenFragments) &&
                message.Details.Contains("grant_type=password"))
            {

                // Message payload will be like: grant_type=password&client_id={3}&username={0}%40{2}&password={1}&scope=openid+offline_access+id_groups+at_groups+rs_admin_server
                string payload = message.Details;
                var parts = payload.Split('&');
                var builder = new StringBuilder();

                for (var index = 0; index < parts.Length; index++)
                {
                    if (parts[index].StartsWith("password="))
                    {
                        parts[index] = "password=XXXX";
                    }
                    builder.Append(parts[index]);
                    if (index != parts.Length - 1)
                        builder.Append("&");
                }
                return builder.ToString();
            }

            // Add New Tenant
            if (message.AbsolutePath.EndsWith(ServiceConfigManager.TenantString) && message.Method == "POST" && message.Details.Contains("credentials"))
            {
                // Message payload will be like
                // {"credentials":{"certificates":[{"encoded":"XXX"},{"encoded":"YYY"}],"privateKey":{"algorithm":"RSA","encoded":"ZZZ"}},"name":"{0}","pasword":"{1}","username":"{2}"}
                var index = message.Details.IndexOf("password");
                var end = message.Details.Substring(index).IndexOf(",");
                var payload = message.Details.Substring(0, index) + "password\":\"XXXX\"" + message.Details.Substring(index + end);
                return payload;
            }

            // Add New User
            if (message.AbsolutePath.EndsWith(ServiceConfigManager.UsersString) && message.Method == "POST" && message.Details.Contains("passwordDetails"))
            {
                // Message payload will be like
                // {"alias":{"domain":"vsphere.local","name":"xyz"},"details":{"description":"Testing 123","email":"def@gmaail.com",
                // "firstName":"XYZ","lastName":"ABC","upn":"xyz@vsphere.local"},"domain":"vsphere.local","name":"xyz","passwordDetails":{"password":"XXXX"}}
                var index = message.Details.IndexOf("passwordDetails");
                var payload = message.Details.Substring(0, index) + "passwordDetails:{\"password\":\"XXXX\"}}";
                return payload;
            }

            // Add New External Domain
            if (message.AbsolutePath.EndsWith(ServiceConfigManager.ProvidersString) && message.Method == "POST" && !message.AbsolutePath.StartsWith("/idm/post"))
            {
                // Message payload will be like
                // {"alias":"","attributesMap":null,"authenticationType":"PASSWORD","connectionStrings":[""],"domainType":"EXTERNAL_DOMAIN","friendlyName":"","groupBaseDN":"","name":"",
                // "password":"","schema":null,"searchTimeOutInSeconds":300,"servicePrincipalName":null,"type":"IDENTITY_STORE_TYPE_LDAP","userBaseDN":"","userMachineAccount":false,"username":""}
                var index = message.Details.IndexOf("password");
                var end = message.Details.Substring(index).IndexOf(",");
                var payload = message.Details.Substring(0, index) + "\"password\"=\"XXXX\"" + message.Details.Substring(index + end);
                return payload;
            }
            return message.Details;

        }
        public HttpTransportCollection GetAll()
        {
            return cachedCollection;
        }
        public IList<HttpTransportItem> GetValues()
        {
            var list = new List<HttpTransportItem>();
            foreach (var item in cachedCollection)
            {
                list.Add((HttpTransportItem)item);
            }
            return list.OrderByDescending(x => x.RequestTimestamp).ToList();
        }
        public void SetAll(HttpTransportCollection httpData)
        {
            if (httpData != null)
                cachedCollection = httpData;
        }
    }
}
