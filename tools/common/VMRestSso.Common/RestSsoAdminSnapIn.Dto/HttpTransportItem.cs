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
using System.ComponentModel;
using System.Runtime.Serialization;
using System.Text;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [TypeConverter(typeof(ExpandableObjectConverter))][Serializable]
    public class HttpTransportItem : IDataContext
    {
        [ReadOnlyAttribute(true), DescriptionAttribute("Unique identitfier for each message trace")]
        public Guid Id { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("Server that serviced the request")]
        public string Server { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("Total time taken from the client to the server and back")]
        public string TimeTaken { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP verb (GET, POST, PUT, DELETE, PATCH) used to make the request to the URI")]
        public string Method { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("The server endpoint corresponding to the requst")]
        public string RequestUri { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP Headers set with the request from client to the server")]
        public HttpTransportRequestHeader RequestHeader { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("Data sent from the client to the server")]
        public string RequestData { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("The timestamp corresponding to the request")]
        public DateTime RequestTimestamp { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("Data sent back from the server to the client")]
        public string ResponseData { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP Headers sent by the server to the client")]
        public HttpTransportResponseHeader ResponseHeader { get; set; }
        [ReadOnlyAttribute(true), DescriptionAttribute("Error returned when the request was processed")]
        public string Error { get; set; }
        public string RequestHeaderAsString { get { return GetRequestHeaderAsString(); } }

        private string GetRequestHeaderAsString()
        {
            var builder = new StringBuilder();
            if (RequestHeader != null)
            {
                builder.AppendFormat("User-Agent : {0}", RequestHeader.UserAgent);
                builder.AppendLine();
                builder.AppendFormat("ContentType : {0}", RequestHeader.ContentType);
                builder.AppendLine();
                builder.AppendFormat("ContentLength : {0}", RequestHeader.ContentLength);
                builder.AppendLine();
                builder.AppendFormat("Host : {0}", RequestHeader.Host);
                builder.AppendLine();
                builder.AppendFormat("Authorization : {0}", RequestHeader.Authorization);
                builder.AppendLine();
            }
            return builder.ToString();
        }
        public string ResponseHeaderAsString { get { return GetResponseHeaderAsString(); } }

        private string GetResponseHeaderAsString()
        {
            var builder = new StringBuilder();
            if (ResponseHeader != null)
            {
                builder.AppendFormat("ContentType : {0}", ResponseHeader.ContentType);
                builder.AppendLine();
                builder.AppendFormat("ContentLength : {0}", ResponseHeader.ContentLength);
                builder.AppendLine();
                builder.AppendFormat("Server : {0}", ResponseHeader.Server);
                builder.AppendLine();
            }
            return builder.ToString();
        }
    }
}
