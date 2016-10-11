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
using System.Linq;

namespace VMDir.Common.Schema
{
    public static class SchemaValueParsers
    {
        public static object SingleValueQuoted(List<string> parts)
        {
            if (parts == null)
                return null;
            return parts.First().Trim('\'');
        }

        public static object SingleValue(List<string> parts)
        {
            if (parts == null)
                return null;
            return parts.First();
        }

        public static object SingleOrMultipleQuoted(List<string> parts)
        {
            if (parts == null)
                return null;
            if (parts[0] == "(")
                return MultipleValuesQuoted(parts);
            else
                return SingleValueQuoted(parts);
        }

        public static object SingleOrMultiple(List<string> parts)
        {
            if (parts == null)
                return null;
            if (parts[0] == "(")
                return MultipleValues(parts);
            else
                return SingleValue(parts);
        }

        public static object SingleOrMultipleDollar(List<string> parts)
        {
            if (parts == null)
                return null;
            parts = SingleOrMultiple(parts) as List<string>;
            if (parts != null)
                parts.RemoveAll(x => x == "$");
            return parts;
        }

        public static object MultipleValues(List<string> parts)
        {
            if (parts == null)
                return null;
            parts.RemoveAt(0);
            parts.RemoveAt(parts.Count - 1);
            return parts;
        }

        public static object MultipleValuesQuoted(List<string> parts)
        {
            if (parts == null)
                return null;
            parts = MultipleValues(parts) as List<string>;
            return parts.Select(x => x.Trim('\'')).ToList();
        }

        public static object MultipleValuesDollar(List<string> parts)
        {
            if (parts == null)
                return null;
            //Remove $ signs
            var newParts = parts.Where(x => x != "$").ToList();
            //Remove '(' and ')'. Check and throw error if does not begin with '(' and end with ')'
            newParts.RemoveAt(0);
            newParts.RemoveAt(newParts.Count - 1);
            return newParts;
        }

        public static object SingleQuotedString(List<string> parts)
        {
            if (parts == null)
                return null;

            var result = string.Join(" ", parts.ToArray());
            return result.Trim('\'');
        }

        public static object IsDefined(List<string> parts)
        {
            return parts != null;
        }
    }
}