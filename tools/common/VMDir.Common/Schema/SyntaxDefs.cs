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

using System.Collections.Generic;
using System.Xml.Serialization;

namespace VMDir.Common.Schema
{
    public class Syntax
    {
        [XmlAttribute]
        public string Name { get; set; }

        [XmlAttribute]
        public string Value { get; set; }

        [XmlAttribute]
        public string Type { get; set; }
    }

    [XmlRoot("SyntaxDefinitions", Namespace = "")]
    public class SyntaxDefs
    {
        [XmlElement("Syntax")]
        public List<Syntax> SyntaxList { get; set; }

        public Syntax this [string index]
        {
            get
            {
                return SyntaxList.Find(x => x.Value == index);
            }
        }

        public Syntax LookupSyntaxByName(string name)
        {
            return SyntaxList.Find(x => x.Name == name);
        }
    }
}
