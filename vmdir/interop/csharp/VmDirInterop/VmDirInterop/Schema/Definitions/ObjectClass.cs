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

namespace VmDirInterop.Schema.Definitions
{
    public class ObjectClass : SchemaDefinition, ISchemaComparable<ObjectClass>
    {
        public String oid { get; private set; }
        public String other { get; private set; }
        public String aux { get; private set; }
        public String must { get; private set; }
        public String may { get; private set; }

        static Regex regex = new Regex(@"^\((?<oid>.*?)NAME '(?<name>[^\s]+)'(?<other>.*?)(?:AUX (?<aux>.*?))?(?:MUST (?<must>.*?))?(?:MAY (?<may>.*?))?\)$");

        public ObjectClass(String defStr) : base(defStr)
        {
            GroupCollection groups = regex.Match(defStr).Groups;
            oid = groups["oid"].Value.Trim();
            name = groups["name"].Value.Trim();
            other = groups["other"].Value.Trim();
            must = groups["must"].Value.Trim();
            may = groups["may"].Value.Trim();
        }

        public int CompareTo(ObjectClass other)
        {
            return base.CompareTo(other);
        }

        public int NameCompareTo(ObjectClass other)
        {
            return base.NameCompareTo(other);
        }

        public int FullCompareTo(ObjectClass other)
        {
            return base.FullCompareTo(other);
        }

        public void MergeContentRule(ContentRule cr)
        {
            StringBuilder sb = new StringBuilder();

            sb.Append("( ").Append(oid);
            sb.Append(" NAME '").Append(name).Append("'");
            sb.Append(" ").Append(other);

            aux = cr.aux;
            if (!String.IsNullOrEmpty(aux))
            {
                sb.Append(" AUX ").Append(aux);
            }

            must = MergeListStr(must, cr.must);
            if (!String.IsNullOrEmpty(must))
            {
                sb.Append(" MUST ").Append(must);
            }

            may = MergeListStr(may, cr.may);
            if (!String.IsNullOrEmpty(may))
            {
                sb.Append(" MAY ").Append(may);
            }

            sb.Append(" )");

            defStr = sb.ToString();
        }

        private String MergeListStr(String org, String add)
        {
            if (String.IsNullOrEmpty(add))
            {
                return org;
            }
            else if (String.IsNullOrEmpty(org))
            {
                return add;
            }
            else
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("( ").Append(StripParenthesis(org));
                sb.Append(" $ ").Append(StripParenthesis(add)).Append(" )");
                return sb.ToString();
            }
        }

        private String StripParenthesis(String str)
        {
            if (str[0] == '(')
            {
                return str.Substring(1, str.Length - 2).Trim();
            }
            else
            {
                return str;
            }
        }
    }
}
