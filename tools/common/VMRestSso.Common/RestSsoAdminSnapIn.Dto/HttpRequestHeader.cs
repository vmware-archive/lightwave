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

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    public class HttpTransportRequestHeader : IDataContext
    {
        private IDictionary<string, string> _headers;
        private string _userAgent;
        private string _contentType;
        private string _contentLength;
        private string _host;
        private string _authorization;

        public HttpTransportRequestHeader()
        {
            _headers = new Dictionary<string, string>();
        }
        public HttpTransportRequestHeader(IDictionary<string, string> headers)
        {
            _headers = headers;
            _userAgent = GetValue("User-Agent");
            _contentType = GetValue("Content-Type");
            _contentLength = GetValue("Content-Length");
            _authorization = GetValue("Authorization");
            _host = GetValue("Host");
        }

        private string GetValue(string key)
        {
            string value;
            _headers.TryGetValue(key, out value);
            return value;
        }

        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP User agent")]
        public string UserAgent
        {
            get
            {
                return _userAgent;
            }
        }

        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP Request Content Type")]
        public string ContentType
        {
            get
            {
                return _contentType;
            }
        }

        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP Request Content Length")]
        public string ContentLength
        {
            get
            {
                return _contentLength;
            }
        }

        [ReadOnlyAttribute(true), DescriptionAttribute("HTTP Request Host")]
        public string Host
        {
            get
            {
                return _host;
            }
        }

        [ReadOnlyAttribute(true), DescriptionAttribute("Server Authorization Token")]
        public string Authorization
        {
            get
            {
                return _authorization;
            }
        }
    }
}