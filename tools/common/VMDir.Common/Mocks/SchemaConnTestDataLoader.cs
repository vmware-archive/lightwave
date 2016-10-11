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
using System.Xml;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Metadata;

//TODO - This file is for testing purposes to provide a mock server with test data. Will be removed.
namespace SchemaConnectionTest.Mocks
{
    public class SchemaConnTestDataLoader
    {
        private IDictionary<String, SchemaConnTestData> testDataDict;

        public SchemaConnTestDataLoader()
        {
            testDataDict = new Dictionary<String, SchemaConnTestData>();
        }

        public void LoadXML(string xmlFilePath)
        {
            SchemaConnTestData testData = new SchemaConnTestData();

            XmlDocument xd = new XmlDocument();
            xd.Load(xmlFilePath);

            // host name
            XmlNode node = xd.SelectSingleNode("//HostName");
            String hostName = node.InnerText;
            testData.HostName = hostName;

            // host status
            node = xd.SelectSingleNode("//Status");
            Boolean status = XmlConvert.ToBoolean(node.InnerText);
            testData.Reachable = status;

            // dse root
            node = xd.SelectSingleNode("//Domain");
            String domain = node.InnerText;
            node = xd.SelectSingleNode("//ServerDn");
            String serverDn = node.InnerText;
            testData.DseRootEntry = new DseRootEntry(domain, serverDn);

            // list of server nodes
            XmlNodeList nodeList = xd.SelectNodes("//ServerEntry");
            IDictionary<String, ServerEntry> serverEntries = new Dictionary<String, ServerEntry>();
            foreach (XmlNode n in nodeList)
            {
                String dn = n.SelectSingleNode(".//DN").InnerText;
                String serverName = n.SelectSingleNode(".//ServerName").InnerText;
                serverEntries.Add(dn, new ServerEntry(dn, serverName));
            }
            testData.ServerEntries = serverEntries;

            // subschema subentry
            nodeList = xd.SelectNodes("//AttributeType");
            IList<AttributeType> attributeTypes = new List<AttributeType>();
            foreach (XmlNode n in nodeList)
            {
                attributeTypes.Add(new AttributeType(n.InnerText));
            }
            nodeList = xd.SelectNodes("//ObjectClass");
            IList<ObjectClass> objectClasses = new List<ObjectClass>();
            foreach (XmlNode n in nodeList)
            {
                objectClasses.Add(new ObjectClass(n.InnerText));
            }
            testData.SubSchemaSubEntry = new SubSchemaSubEntry(attributeTypes, objectClasses);

            // list of attribute schema entries
            nodeList = xd.SelectNodes("//AttributeSchemaEntry");
            IList<SchemaEntry> attributeSchemaEntries = new List<SchemaEntry>();
            foreach (XmlNode n in nodeList)
            {
                String defName = n.SelectSingleNode(".//Name").InnerText;
                IList<AttributeMetadata> metadataList = new List<AttributeMetadata>();
                foreach (XmlNode _n in n.SelectNodes(".//Value"))
                {
                    metadataList.Add(new AttributeMetadata(_n.InnerText));
                }
                attributeSchemaEntries.Add(new SchemaEntry(defName, metadataList));
            }
            testData.AttributeSchemaEntries = new SchemaComparableList<SchemaEntry>(attributeSchemaEntries);

            // list of class schema entries
            nodeList = xd.SelectNodes("//ClassSchemaEntry");
            IList<SchemaEntry> classSchemaEntries = new List<SchemaEntry>();
            foreach (XmlNode n in nodeList)
            {
                String defName = n.SelectSingleNode(".//Name").InnerText;
                IList<AttributeMetadata> metadataList = new List<AttributeMetadata>();
                foreach (XmlNode _n in n.SelectNodes(".//Value"))
                {
                    metadataList.Add(new AttributeMetadata(_n.InnerText));
                }
                classSchemaEntries.Add(new SchemaEntry(defName, metadataList));
            }
            testData.ClassSchemaEntries = new SchemaComparableList<SchemaEntry>(classSchemaEntries);

            // store test data
            testDataDict.Add(hostName, testData);
        }

        public SchemaConnTestData GetTestData(String hostName)
        {
            SchemaConnTestData testData = null;
            if (testDataDict.ContainsKey(hostName))
            {
                testData = testDataDict[hostName];
            }
            return testData;
        }
    }
}
