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
using System.Windows.Forms;
using System.Reflection;
using System.Drawing;
using System.IO;
using System.Xml.Serialization;
using System.DirectoryServices.Protocols;
using Microsoft.ManagementConsole.Advanced;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;

namespace VMDirSnapIn.Services
{
    public static class MiscUtilsService
    {
        public static void CheckedExec(System.Action fn)
        {
            try
            {
                fn();
            }
            catch (Exception exp)
            {
                ShowError(exp);
            }
        }
        public static object GetInstanceFromType(string t)
        {
            var type = Type.GetType(t);
            if (type == null) return "";
            if (type.GetConstructor(Type.EmptyTypes) == null)
            {
                if (t == "System.DateTime") return DateTime.Now;
                return "";
            }
            return Activator.CreateInstance(type);
        }

        public static Image GetResourceImage(string name)
        {
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                return new Bitmap(stream);
            }
        }

        public static string GetResourceXML(string name)
        {
            using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                using (StreamReader reader = new StreamReader(stream))
                {
                    return reader.ReadToEnd();
                }
            }
        }

        public static DialogResult ShowError(Exception exp)
        {
            string error = exp.Message;
            var vmdirExp = exp as VMDirException;
            if(vmdirExp != null)
                error = string.Format("{0}, Error Code: {1}", error, vmdirExp.ErrorCode);
            else if (exp is DirectoryOperationException)
                error = (exp as DirectoryOperationException).Response.ErrorMessage;
            return ShowError(error);
        }

        public static DialogResult ShowError(string error)
        {
            return MessageBox.Show(error);
        }

        public static DialogResult ShowError(string text, string caption, MessageBoxButtons btn)
        {
            return MessageBox.Show(text, caption, btn);
        }

        public static T GetDTOFromXML<T>(string xml)
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

        public static bool Confirm(string message)
        {
            var msgParams = new MessageBoxParameters
            {
                Buttons = MessageBoxButtons.YesNo,
                Icon = MessageBoxIcon.Question,
                Text = message
            };

            return VMDirEnvironment.Instance.SnapIn.Console.ShowDialog(msgParams) == DialogResult.Yes;
        }
        public static void ConvertToKVData(Dictionary<string, VMDirBagItem> _properties, List<KeyValuePair<string,string>> _kvData)
        {
            MiscUtilsService.CheckedExec(delegate
            {
                _kvData.Clear();
                foreach (var entry in _properties)
                {
                    object value = entry.Value.Value;
                    if (value != null)
                    {
                        Type valueType = value.GetType();
                        if (valueType.IsArray)
                        {
                            LdapValue[] arr = value as LdapValue[];
                            foreach (var arrayElement in arr)
                            {
                                _kvData.Add(new KeyValuePair<string, string>(entry.Key, arrayElement.StringValue));
                            }
                        }
                        else
                        {
                            var LdapEntry = (LdapValue)value;
                            _kvData.Add(new KeyValuePair<string, string>(entry.Key, value.ToString()));
                        }
                    }
                    else
                        _kvData.Add(new KeyValuePair<string, string>(entry.Key, string.Empty));
                }
                _kvData.Sort(delegate(KeyValuePair<string, string> x, KeyValuePair<string, string> y)
                {
                    return x.Key.CompareTo(y.Key);
                });
            });
        }
    }
}
