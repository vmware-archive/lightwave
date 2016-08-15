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
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using VMDirInterop;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VmDirInterop.Schema.Constants;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Diffs;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Exceptions;
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Metadata;
using VmDirInterop.Schema.Utils;
using System.Xml;

namespace VmDirInterop.Schema
{
    public class SchemaConnection : ISchemaConnection
    {
        private IEntryFetcherFactory entryFetcherFactory;
        private IEntryFetcher baseEntryFetcher;
        private IDictionary<String, IEntryFetcher> entryFetchers;
        private String baseServerName;

        public SchemaConnection(String baseHost, String upn, String passwd)
            : this(new EntryFetcherFactory(), baseHost, upn, passwd)
        {
        }

        public SchemaConnection(IEntryFetcherFactory entryFetcherFactory, String baseHost, String upn, String passwd)
        {
            this.entryFetcherFactory = entryFetcherFactory;
            baseEntryFetcher = entryFetcherFactory.CreateEntryFetcher(baseHost, upn, passwd);
            if (baseEntryFetcher == null)
            {
                throw new SchemaConnectionException("Base server is not reachable");
            }

            entryFetchers = new Dictionary<String, IEntryFetcher>();

            DseRootEntry dseRootEntry = baseEntryFetcher.GetDseRootEntry();
            ServerEntry serverEntry = baseEntryFetcher.GetServerEntry(dseRootEntry.serverDn);
            baseServerName = serverEntry.serverName;

            RefreshSchemaConnection(upn, passwd);
        }

        public String GetBaseServerName()
        {
            return baseServerName;
        }

        public IDictionary<String, Boolean> GetAllServerStatus()
        {
            IDictionary<String, Boolean> reachable = new Dictionary<String, Boolean>();
            foreach (KeyValuePair<String, IEntryFetcher> p in entryFetchers)
            {
                reachable.Add(p.Key, p.Value != null);
            }
            return reachable;
        }

        public IDictionary<String, SchemaDefinitionDiff> GetAllSchemaDefinitionDiffs()
        {
            IDictionary<String, SchemaDefinitionDiff> diffs =
                new Dictionary<String, SchemaDefinitionDiff>();

            SubSchemaSubEntry baseSubSchema = baseEntryFetcher.GetSubSchemaSubEntry();
            foreach (KeyValuePair<String, IEntryFetcher> p in entryFetchers)
            {
                String serverName = p.Key;
                IEntryFetcher fetcher = p.Value;
                SchemaDefinitionDiff diff = null;
                if (fetcher != null)
                {
                    SubSchemaSubEntry otherSubSchema = fetcher.GetSubSchemaSubEntry();
                    diff = new SchemaDefinitionDiff(baseSubSchema, otherSubSchema);
                }
                diffs.Add(serverName, diff);
            }
            return diffs;
        }

        public IDictionary<String, SchemaMetadataDiff> GetAllSchemaMetadataDiffs()
        {
            IDictionary<String, SchemaMetadataDiff> diffs
                = new Dictionary<String, SchemaMetadataDiff>();

            SchemaComparableList<SchemaEntry> baseAtEntries =
                baseEntryFetcher.GetAttributeSchemaEntries();
            SchemaComparableList<SchemaEntry> baseOcEntries =
                baseEntryFetcher.GetClassSchemaEntries();

            foreach (KeyValuePair<String, IEntryFetcher> p in entryFetchers)
            {
                String serverName = p.Key;
                IEntryFetcher fetcher = p.Value;
                SchemaMetadataDiff diff = null;
                if (fetcher != null)
                {
                    SchemaComparableList<SchemaEntry> otherAtEntries =
                        fetcher.GetAttributeSchemaEntries();
                    SchemaComparableList<SchemaEntry> otherOcEntries =
                        fetcher.GetClassSchemaEntries();

                    diff = new SchemaMetadataDiff(
                        baseAtEntries, baseOcEntries, otherAtEntries, otherOcEntries);
                }
                diffs.Add(serverName, diff);
            }
            return diffs;
        }

        public void RefreshSchemaConnection(String upn, String passwd)
        {
            IList<ServerEntry> serverEntries = baseEntryFetcher.GetServerEntries();
            entryFetchers.Clear();

            Mutex mutex = new Mutex();
            Parallel.ForEach(serverEntries, (e) =>
            {
                if (String.Compare(baseServerName, e.serverName) != 0)
                {
                    IEntryFetcher entryFetcher =
                        entryFetcherFactory.CreateEntryFetcher(
                        e.serverName, upn, passwd);

                    mutex.WaitOne();
                    entryFetchers.Add(e.serverName, entryFetcher);
                    mutex.ReleaseMutex();
                }
            });
        }

        public void ExportToXML(String filepath)
        {
            using (XmlTextWriter writer = new XmlTextWriter(filepath, null))
            {
                writer.Formatting = Formatting.Indented;
                writer.WriteStartDocument();
                writer.WriteStartElement("Node");

                // host name
                writer.WriteElementString("HostName", baseServerName);

                // host status
                writer.WriteElementString("Status", "1");

                // dse root
                DseRootEntry dseRootEntry = baseEntryFetcher.GetDseRootEntry();
                writer.WriteStartElement("DseRootEntry");
                writer.WriteElementString("Domain", dseRootEntry.domain);
                writer.WriteElementString("ServerDn", dseRootEntry.serverDn);
                writer.WriteEndElement();

                // list of server nodes
                IList<ServerEntry> serverEntries = baseEntryFetcher.GetServerEntries();
                writer.WriteStartElement("ServerEntries");
                foreach (ServerEntry e in serverEntries)
                {
                    writer.WriteStartElement("ServerEntry");
                    writer.WriteElementString("DN", e.dn);
                    writer.WriteElementString("ServerName", e.serverName);
                    writer.WriteEndElement();
                }
                writer.WriteEndElement();

                // subschema subentry
                SubSchemaSubEntry baseSubSchema = baseEntryFetcher.GetSubSchemaSubEntry();
                writer.WriteStartElement("SubSchemaSubEntry");
                writer.WriteStartElement("AttributeTypes");
                foreach (AttributeType at in baseSubSchema.GetAttributeTypeList())
                {
                    writer.WriteElementString("AttributeType", at.ToString());
                }
                writer.WriteEndElement();
                writer.WriteStartElement("ObjectClasses");
                foreach (ObjectClass oc in baseSubSchema.GetObjectClassList())
                {
                    writer.WriteElementString("ObjectClass", oc.ToString());
                }
                writer.WriteEndElement();
                writer.WriteEndElement();

                // list of attribute schema entries
                SchemaComparableList<SchemaEntry> baseAtEntries =
                baseEntryFetcher.GetAttributeSchemaEntries();
                SchemaComparableList<SchemaEntry> baseOcEntries =
                    baseEntryFetcher.GetClassSchemaEntries();
                writer.WriteStartElement("SchemaEntries");
                writer.WriteStartElement("AttributeSchemaEntries");
                foreach (SchemaEntry ae in baseAtEntries)
                {
                    writer.WriteStartElement("AttributeSchemaEntry");
                    writer.WriteElementString("Name", ae.defName);
                    writer.WriteStartElement("AttributeMetadata");
                    foreach (AttributeMetadata m in ae.GetMetadataList())
                    {
                        writer.WriteElementString("Value", m.raw);
                    }
                    writer.WriteEndElement();
                    writer.WriteEndElement();
                }
                writer.WriteEndElement();

                // list of class schema entries
                writer.WriteStartElement("ClassSchemaEntries");
                foreach (SchemaEntry ce in baseOcEntries)
                {
                    writer.WriteStartElement("ClassSchemaEntry");
                    writer.WriteElementString("Name", ce.defName);
                    writer.WriteStartElement("AttributeMetadata");
                    foreach (AttributeMetadata m in ce.GetMetadataList())
                    {
                        writer.WriteElementString("Value", m.raw);
                    }
                    writer.WriteEndElement();
                    writer.WriteEndElement();
                }
                writer.WriteEndElement();
                writer.WriteEndElement();

                // end node
                writer.WriteEndElement();
                writer.WriteEndDocument();
            }
        }
    }

    struct timeval
    {
        long tv_sec;
        long tv_usec;

        public timeval(long sec, long usec)
        {
            tv_sec = sec;
            tv_usec = usec;
        }
    }
}
