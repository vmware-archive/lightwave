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
using System.IO;
using System.Xml.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Cache;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Persistence;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Data.Storage
{
    public class HttpTransportLocalFileStorage : IPersistentStorage<HttpTransportCollection>
    {
        private readonly string _fileName;

        public HttpTransportLocalFileStorage(string fileName)
        {
            _fileName = fileName;
        }

        public void Save(HttpTransportCollection store)
        {
            using (var ms = new MemoryStream())
            {
                var xmlSerializer = new XmlSerializer(typeof(HttpTransportCollection));
                xmlSerializer.Serialize(ms, store);
                File.WriteAllBytes(_fileName, ms.ToArray());
            }
        }

        public HttpTransportCollection Load()
        {
            if (!File.Exists(_fileName))
            {
                return null;
            }

            try
            {
                using (var ms = new MemoryStream())
                {
                    var bytes = File.ReadAllBytes(_fileName);
                    ms.Write(bytes, 0, bytes.Length);
                    ms.Seek(0, SeekOrigin.Begin);
                    var xmlSerializer = new XmlSerializer(typeof(HttpTransportCollection));
                    return xmlSerializer.Deserialize(ms) as HttpTransportCollection;
                }
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}
