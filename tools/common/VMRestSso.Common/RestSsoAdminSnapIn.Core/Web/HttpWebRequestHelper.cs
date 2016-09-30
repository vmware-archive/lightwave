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

using System.IO;
using System.Net;
using System.Collections.Generic;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Web
{
    public class HttpWebRequestHelper
    {
        public static HttpResponse GetResponseFromServer(WebRequest request)
        {
            using (var response = (WebResponse)request.GetResponse())
            {
                var httpResponse = new HttpResponse(response);
                using (var dataStream = response.GetResponseStream())
                {
                    if (dataStream != null)
                    {
                        using (var reader = new StreamReader(dataStream))
                        {
                            var data = reader.ReadToEnd();
                            httpResponse.Data = data;
                        }
                    }
                }
                return httpResponse;
            }
        }

        public static void WriteStringToRequestStream(WebRequest request, string data)
        {
            using (var streamWriter = new StreamWriter(request.GetRequestStream()))
            {
                streamWriter.Write(data);
                streamWriter.Flush();
            }
        }

        public static IDictionary<string, string> GetHeaders(WebHeaderCollection webHeaders)
        {
            var headers = new Dictionary<string, string>();
            foreach (string key in webHeaders.AllKeys)
            {
                headers.Add(key, webHeaders[key]);
            }
            return headers;
        }
    }
}
