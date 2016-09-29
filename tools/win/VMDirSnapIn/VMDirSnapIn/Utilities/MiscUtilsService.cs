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
using VMDir.Common;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDirInterop.Interfaces;

namespace VMDirSnapIn.Utilities
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
                VMDirEnvironment.Instance.Logger.LogException(exp);
                ShowError(exp);
            }
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
            if (vmdirExp != null)
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

        public static string GetObjectClass(ILdapEntry entry)
        {
            var values = entry.getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS);
            return values[(values.Count() - 1)].StringValue;
        }

        internal static int GetImgIndx(List<string> objectClass)
        {
            if (objectClass.Contains(VMDirConstants.USER_OC))
                return (int)VMDirIconIndex.User;
            else if (objectClass.Contains(VMDirConstants.GROUP_OC))
                return (int)VMDirIconIndex.Group;
            else
                return (int)VMDirIconIndex.Object;
        }
    }
}
