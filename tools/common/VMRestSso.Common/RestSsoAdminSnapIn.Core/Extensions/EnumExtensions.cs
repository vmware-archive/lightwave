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
using System.Reflection;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions
{
    public static class EnumExtensions
    {
        public static string GetDescription(this Enum enumerator)
        {
            Type type = enumerator.GetType();

            MemberInfo[] memberInfo = type.GetMember(enumerator.ToString());

            if (memberInfo != null && memberInfo.Length > 0)
            {
                object[] attributes = memberInfo[0].GetCustomAttributes(typeof(DescriptionAttribute), false);

                if (attributes != null && attributes.Length > 0)
                {
                    return ((DescriptionAttribute)attributes[0]).Description;
                }
            }

            return enumerator.ToString();
        }

        public static Dictionary<int, string> ToDictionary(this Enum enumerator)
        {
            var dictionary = new Dictionary<int, string>();
            Type type = enumerator.GetType();
            int index = 0;
            foreach (Enum item in Enum.GetValues(type))
            {
                var description = item.GetDescription();
                dictionary.Add(index++, description);
            }
            return dictionary;
        }

        public static List<string> ToList(this Enum enumerator)
        {
            var list = new List<string>();
            Type type = enumerator.GetType();
            foreach (Enum item in Enum.GetValues(type))
            {                
                list.Add(item.ToString());
            }
            return list;
        }

        public static List<KeyValuePair> ToKeyValueList(this Enum enumerator)
        {
            var list = new List<KeyValuePair>();
            Type type = enumerator.GetType();
            foreach (Enum item in Enum.GetValues(type))
            {
                var description = item.GetDescription();
                var keyValue = new KeyValuePair { Key = item.ToString(), Value = description };
                list.Add(keyValue);
            }
            return list;
        }

        public static Enum GetByDescription(this Enum enumerator, string description)
        {            
            Type type = enumerator.GetType();
            Enum value = enumerator;
            foreach (Enum item in Enum.GetValues(type))
            {
                var desc = item.GetDescription();
                if (desc == description)
                {
                    value = item;
                    break;
                }
            }
            return value;
        }
    }

    public class KeyValuePair
    {
        public string Key { get; set; }
        public string Value { get; set; }
    }
}
