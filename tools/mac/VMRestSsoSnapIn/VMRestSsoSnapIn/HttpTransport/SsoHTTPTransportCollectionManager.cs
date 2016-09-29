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

namespace Vmware.Tools.RestSsoAdminSnapIn.HttpTransport
{
    public class SsoHttpTransportCollectionManager
    {
        readonly SsoHttpTransportCollection _allTransport = new SsoHttpTransportCollection();
        SsoHttpTransport _currentTransport;
        public SsoHttpTransportCollection AllTransport { get { return _allTransport; } }
        public void AddRequestHeader(SsoHttpHeader header)
        {
            if (_currentTransport == null)
                _currentTransport = new SsoHttpTransport();
            _currentTransport.RequestHeader = header;
            _currentTransport.Method = header.Method;
        }
        public void AddResponseHeader(SsoHttpHeader header)
        {
            if (_currentTransport != null)
                _currentTransport.ResponseHeader = header;
        }
        public void AddRequest(string request)
        {
            if (_currentTransport != null)
                _currentTransport.Request = request;
        }
        public void AddResponse(string response)
        {
            if (_currentTransport != null)
            {
                _currentTransport.Response = response;
                Flush();
            }
        }
        public void AddError(string error)
        {
            if (_currentTransport != null)
            {
                _currentTransport.Error = error;
                Flush();
            }
        }
        void Flush()
        {
            _allTransport.Add(_currentTransport);
            if (_currentTransport.ResponseHeader != null && _currentTransport.RequestHeader != null)
            {
                var timeDiff = _currentTransport.ResponseHeader.TimeStamp - _currentTransport.RequestHeader.TimeStamp;
                _currentTransport.TimeTaken = string.Format("{0}:{1}s", timeDiff.Seconds, timeDiff.Milliseconds);
            }
            _currentTransport = null;
        }
    }
}
