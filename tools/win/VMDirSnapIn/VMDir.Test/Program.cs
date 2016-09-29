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
using System.DirectoryServices.Protocols;
using System.Windows.Forms;
using VMDirSnapIn;
using VMDirSnapIn.DTO;
using VMDirSnapIn.UI;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Linq;
using VMDir.Common.Schema;
using System.Xml;
using VMDirSnapIn.Services;
using VMDir.Common.DTO;

namespace VMDir.Test
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            DoLdap();
        }

        static void DoLdap()
        {
            var dto = ShowSelectComputerUI();
            if (dto == null)
                return;

            dto.Connection = new VMDirConnection(dto.Server, dto.BindDN, dto.Password);
            dto.Connection.CreateConnection();
            var ocd = dto.Connection.SchemaManager.GetObjectClass("organization");
            dto.Connection.SchemaManager.GetRequiredAttributes("vmIdentity-Group");
            var cr = dto.Connection.SchemaManager.GetContentRule("organization");

            string dn = "CN=a,CN=Users,DC=vsphere,DC=local";
            var pp = new AttributeEditorPropertyPage(dn, dto);
            //new SchemaBrowser(dto).ShowDialog();
        }

        static void CreateForm(string objectClass, VMDirServerDTO dto)
        {
            var frm = new CreateForm(objectClass, dto);
            if (frm.ShowDialog() == DialogResult.Cancel)
                return;

            var attributes = frm.Attributes.Select(x => LdapTypesService.MakeAttribute(x)).ToArray();

            var cnVal = frm.Attributes.First(x => x.Key == "cn").Value.Value;
            string dn = string.Format("cn={0},{1}", cnVal, "dc=vsphere,dc=local");
            dto.Connection.Add(dn, attributes);
        }

        static void ParseObjectClass(string attr)
        {
            var defs = new List<SchemaComponentDef>
            {
                new SchemaComponentDef{Name="NAME", Parser=SchemaValueParsers.SingleValueQuoted},
                new SchemaComponentDef{Name="SUP", Parser=SchemaValueParsers.SingleValue},
                new SchemaComponentDef{Name="STRUCTURAL", Parser=SchemaValueParsers.IsDefined},
                new SchemaComponentDef{Name="MUST", Parser=SchemaValueParsers.MultipleValuesDollar},
                new SchemaComponentDef{Name="MAY", Parser=SchemaValueParsers.MultipleValuesDollar},
                new SchemaComponentDef{Name="AUXILIARY", Parser=SchemaValueParsers.IsDefined},
            };
            ShowDict(Parse(defs, attr));
        }

        static void ParseAttributeTypes(string attr)
        {
            var defs = new List<SchemaComponentDef>
            {
                new SchemaComponentDef{Name="NAME", Parser=SchemaValueParsers.SingleOrMultipleQuoted},
                new SchemaComponentDef{Name="DESC", Parser=SchemaValueParsers.SingleQuotedString},
                new SchemaComponentDef{Name="EQUALITY", Parser=SchemaValueParsers.SingleValue},
                new SchemaComponentDef{Name="SUBSTR", Parser=SchemaValueParsers.SingleValue},
                new SchemaComponentDef{Name="SYNTAX", Parser=SchemaValueParsers.SingleValue},
            };
            ShowDict(Parse(defs, attr));
        }

        static void ShowDict(Dictionary<string, object> dict)
        {
            foreach (var entry in dict)
            {
                string result = "";
                var val = entry.Value;
                if (val == null)
                    result = "{NULL}";
                else if (val is string)
                    result = val.ToString();
                else
                    result = string.Join(",", (val as List<string>).ToArray());

                Console.WriteLine(string.Format("{0}: {1}", entry.Key, result));
            }
        }

        static Dictionary<string, object> Parse(List<SchemaComponentDef> defs, string attr)
        {
            var dict = new Dictionary<string, object>();

            var bits = attr.Split(' ');
            var trimmedBits = bits.Where(x => !string.IsNullOrEmpty(x)).ToList();
            
            var indices = new List<int>();
            foreach (var entry in defs)
            {
                int index = trimmedBits.IndexOf(entry.Name);
                entry.IndexRange.Start = index;
                indices.Add(index);
            }
            indices.Add(trimmedBits.Count-1);
            indices.Sort();

            defs.ForEach(x=>x.IndexRange.End = indices[indices.IndexOf(x.IndexRange.Start) + 1]);
            foreach (var entry in defs)
            {
                int start = entry.IndexRange.Start + 1;
                int end = entry.IndexRange.End;
                if (entry.Parser != null)
                {
                    if (start == 0)
                        dict[entry.Name] = entry.Parser(null);
                    else
                        dict[entry.Name] = entry.Parser(trimmedBits.GetRange(start, end - start));
                }
/*
                var result = new List<string>();
                if (trimmedBits[start] == "(")
                {
                    ++start; --end;
                    for (int i = start; i < end; ++i)
                        result.Add(trimmedBits[i].Trim('\''));
                }
                else
                {
                    var arrayVals = trimmedBits.GetRange(start, end - start).ToArray();
                    result.Add(string.Join(" ", arrayVals));
                }
                dict[entry.Name] = result;*/
            }
            return dict;
        }

        public static VMDirServerDTO ShowSelectComputerUI()
        {
            var ui = new frmConnectToServer();
            //ui.txtDirectoryServer.Text = "192.168.2.9:11711";
            //ui.txtDirectoryServer.Text = "10.118.72.132:11711";
            if (ui.ShowDialog() == DialogResult.OK)
                return ui.ServerDTO;
            return null;
        }

        static void Test(VMDirConnection conn)
        {
            try
            {
                conn.CreateConnection();
                var baseDN = "cn=aggregate,cn=schemacontext";

                var sb = new StringBuilder();
                var response = conn.Search(baseDN, "(objectClass=*)", null, SearchScope.Subtree);
                foreach (SearchResultEntry entry in response.Entries)
                {
                    foreach (DictionaryEntry attrib in entry.Attributes)
                    {
                        sb.AppendFormat(">>>>>>>>>>>>>>>>>{0}<<<<<<<<<<<<<<<", attrib.Key).AppendLine();

                        var val = attrib.Value as DirectoryAttribute;
                        bool hasMultiple = val.Count > 1;
                        if (!hasMultiple)
                        {
                            string valString = "";
                            if (val.Count == 1)
                                valString = val[0].ToString();
                        }
                        else
                        {
                            var values = new List<string>();
                            int count = val.Count;
                            for (int i = 0; i < count; ++i)
                            {
                                object o = val[i];
                                string type = o.GetType().ToString();
                                sb.AppendFormat("{0}: {1}", type, o.ToString()).AppendLine();
                                values.Add(val[i].ToString());
                            }
                        }
                    }
                }
                File.WriteAllText("c:\\temp\\dir2.txt", sb.ToString());
            }
            catch (Exception exp)
            {
                MessageBox.Show(exp.ToString());
            }
        }

        static void Test2()
        {
            var attrTypes = new List<string>();
            attrTypes.Add("( 0.9.2342.19200300.100.1.1 NAME 'uid' SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 EQUALITY caseIgnoreMatch SUBSTR caseIgnoreSubstringsMatch )");
            attrTypes.Add("( 1.2.840.113549.1.9.1 NAME ( 'email' 'emailAddress' 'pkcs9email' ) DESC 'RFC3280: legacy attribute for email addresses in DNs' EQUALITY caseIgnoreIA5Match SUBSTR caseIgnoreIA5SubstringsMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.26{128} )");
            var a = new AttributeTypeManager(attrTypes);

            var objectClasses = new List<string>();
            objectClasses.Add("( 0.9.2342.19200300.100.4.7 NAME 'room' SUP top STRUCTURAL MUST ( cn ) MAY ( description $ telephoneNumber $ seeAlso $ location $ roomNumber ) )");
            var b = new ObjectClassManager(objectClasses);
            //ParseObjectClass(attr);
        }
    }
}
