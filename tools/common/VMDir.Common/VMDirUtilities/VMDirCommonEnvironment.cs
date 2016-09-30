/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;
using System.Xml.Serialization;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using System.Linq;

namespace VMDir.Common.VMDirUtilities
{
    public class VMDirCommonEnvironment
    {
        private SyntaxDefs _syntaxDefs;
        private Dictionary<string, AttributeHelpDTO> _attrHelpDict;
        static VMDirCommonEnvironment _instance;
        public static VMDirCommonEnvironment Instance {
            get {
                if (_instance == null)
                    _instance = new VMDirCommonEnvironment ();
                return _instance;
            }
        }

        public SyntaxDefs SyntaxDefs {
            get {
                try {
                    if (_syntaxDefs == null) {
                        string xml = GetResourceXML ("VMDir.Common.Resources.LDAPDefs.xml");
                        _syntaxDefs = GetDTOFromXML<SyntaxDefs> (xml);
                    }
                    return _syntaxDefs;
                } catch (Exception) {
                    return new SyntaxDefs();
                }
            }
        }

        public Dictionary<string, AttributeHelpDTO> AttrHelpDict
        {
            get
            {
                try
                {
                    if (_attrHelpDict == null)
                    {
                        string xml = GetResourceXML("VMDir.Common.Resources.LDAPDefsHelp.xml");
                        var attrlst = GetDTOFromXML<AttributeHelpListDTO>(xml);
                        if (attrlst != null)
                            _attrHelpDict = attrlst.AttrHelpList.ToDictionary(x => x.Value);
                        else
                            _attrHelpDict = new Dictionary<string, AttributeHelpDTO>();
                    }
                    return _attrHelpDict;
                }
                catch (Exception)
                {
                    return new Dictionary<string, AttributeHelpDTO>();
                }
            }
        }

        public static string GetResourceXML (string name)
        {
            using (Stream stream = Assembly.GetExecutingAssembly ().GetManifestResourceStream (name)) {
                if (stream != null) {
                    using (var reader = new StreamReader (stream)) {
                        return reader.ReadToEnd ();
                    }
                } else {
                    return null;
                }
            }
        }

        public static T GetDTOFromXML<T> (string xml)
        {
            using (var ms = new MemoryStream ()) {
                var bytes = Encoding.UTF8.GetBytes (xml);
                ms.Write (bytes, 0, bytes.Length);
                ms.Seek (0, SeekOrigin.Begin);

                var serializer = new XmlSerializer (typeof(T));
                return (T)serializer.Deserialize (ms);
            }
        }
    }
}
