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
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization
{
    public class XmlConvert
    {
        public static T Deserialize<T>(string xml)
        {
            using (var ms = new MemoryStream())
            {
                var bytes = Encoding.UTF8.GetBytes(xml);
                ms.Write(bytes, 0, bytes.Length);
                ms.Seek(0, SeekOrigin.Begin);

                var serializer = new XmlSerializer(typeof(T));
                return (T)serializer.Deserialize(ms);
            }
        }

        public static string Serialize<T>(T data)
        {
            var serializer = new XmlSerializer(typeof(T));
            var settings = new XmlWriterSettings();
            settings.Encoding = new UnicodeEncoding(false, false); 
            settings.Indent = true;
            settings.OmitXmlDeclaration = true;

            using(var textWriter = new StringWriter()) {
                using(var xmlWriter = XmlWriter.Create(textWriter, settings)) {
                    serializer.Serialize(xmlWriter, data);
                }
                return textWriter.ToString();
            }
        }
    }
}
