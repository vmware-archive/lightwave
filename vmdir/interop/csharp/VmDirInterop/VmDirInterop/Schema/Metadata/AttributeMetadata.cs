/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using VmDirInterop.Schema.Interfaces;

namespace VmDirInterop.Schema.Metadata
{
    public class AttributeMetadata : ISchemaComparable<AttributeMetadata>
    {
        public String raw { get; protected set; }
        public String attr { get; protected set; }
        public String metadata { get; protected set; }

        static Regex regex = new Regex(@"^(?<attr>[^:]+):\d+:(?<metadata>.*)$");

        public AttributeMetadata(String metadataString)
        {
            raw = metadataString;
            GroupCollection groups = regex.Match(metadataString).Groups;
            attr = groups["attr"].Value.Trim();
            metadata = attr + ":" + groups["metadata"].Value.Trim();
        }

        public int CompareTo(AttributeMetadata other)
        {
            return this.NameCompareTo(other);
        }

        public int NameCompareTo(AttributeMetadata other)
        {
            return String.Compare(this.attr, other.attr);
        }

        public int FullCompareTo(AttributeMetadata other)
        {
            return String.Compare(this.metadata, other.metadata);
        }

        public override string ToString()
        {
            return metadata;
        }
    }
}
