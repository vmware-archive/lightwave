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
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Entries;
using VMDirInterop.Interfaces;
using VmDirInterop.Schema.Constants;
using VmDirInterop.Schema.Exceptions;

namespace VmDirInterop.Schema
{
    public class EntryFetcher : IEntryFetcher
    {
        private ILdapConnection ldapConn;

        public EntryFetcher(ILdapConnection ldapConn)
        {
            this.ldapConn = ldapConn;
        }

        public DseRootEntry GetDseRootEntry()
        {
            String dn = null;
            int scope = SearchScope.BASE;
            String filter = FilterStr.ASTERISK;
            String[] attrs = new string[] {
                AttributeConstants.NAMING_CONTEXT,
                AttributeConstants.SERVER_NAME,
                null
            };

            ILdapMessage msg = SimpleSearch(dn, scope, filter, attrs);
            DseRootEntry entry = new DseRootEntry(msg.GetEntries()[0]);
            return entry;
        }

        public ServerEntry GetServerEntry(String dn)
        {
            int scope = SearchScope.BASE;
            String filter = FilterStr.VMW_DIR_SERVER;
            String[] attrs = new string[] {
                AttributeConstants.CN, null
            };

            ILdapMessage msg = SimpleSearch(dn, scope, filter, attrs);
            ServerEntry entry = new ServerEntry(msg.GetEntries()[0]);
            return entry;
        }

        public SubSchemaSubEntry GetSubSchemaSubEntry()
        {
            String dn = DnConstants.SUB_SCHEMA_SUB_ENTRY;
            int scope = SearchScope.BASE;
            String filter = FilterStr.SUB_SCHEMA;
            String[] attrs = new string[] {
                AttributeConstants.ATTRIBUTE_TYPES, AttributeConstants.OBJECT_CLASSES, AttributeConstants.DIT_CONTENT_RULES, null
            };

            ILdapMessage msg = SimpleSearch(dn, scope, filter, attrs);
            SubSchemaSubEntry entry = new SubSchemaSubEntry(msg.GetEntries()[0]);
            return entry;
        }

        public IList<ServerEntry> GetServerEntries()
        {
            IList<ServerEntry> entries = new List<ServerEntry>();

            String dn = string.Format(
                "{0},{1},{2}",
                CnConstants.SITES, CnConstants.CONFIGURATION,
                GetDseRootEntry().domain);

            int scope = SearchScope.SUBTREE;
            String filter = FilterStr.VMW_DIR_SERVER;
            String[] attrs = new string[] {
                AttributeConstants.DN, null
            };

            ILdapMessage msg = SimpleSearch(dn, scope, filter, attrs);
            foreach (ILdapEntry e in msg.GetEntries())
            {
                entries.Add(GetServerEntry(e.getDN()));
            }
            return entries;
        }

        public SchemaComparableList<SchemaEntry> GetAttributeSchemaEntries()
        {
            IList<SchemaEntry> entries = GetSchemaEntries(FilterStr.ATTRIBUTE_SCHEMA);
            return new SchemaComparableList<SchemaEntry>(entries);
        }

        public SchemaComparableList<SchemaEntry> GetClassSchemaEntries()
        {
            IList<SchemaEntry> entries = GetSchemaEntries(FilterStr.CLASS_SCHEMA);
            return new SchemaComparableList<SchemaEntry>(entries);
        }

        private IList<SchemaEntry> GetSchemaEntries(String filter)
        {
            IList<SchemaEntry> entries = new List<SchemaEntry>();

            String dn = DnConstants.SCHEMA_CONTEXT;
            int scope = SearchScope.ONELEVEL;
            String[] attrs = new string[] {
                AttributeConstants.CN,
                AttributeConstants.ATTRIBUTE_METADATA,
                null
            };

            ILdapMessage msg = SimpleSearch(dn, scope, filter, attrs);
            foreach (ILdapEntry e in msg.GetEntries())
            {
                entries.Add(new SchemaEntry(e));
            }
            return entries;
        }

        private ILdapMessage SimpleSearch(String dn, int scope, String filter, String[] attrs)
        {
            try
            {
                return ldapConn.LdapSearchExtS(
                    dn, scope, filter, attrs, 0, IntPtr.Zero, 0);
            }
            catch (Exception e)
            {
                throw new SchemaConnectionException(
                    String.Format("LDAP Search failed (dn=\"{0}\" scope={1} filter=\"{2}\")",
                    dn, scope, filter),
                    e);
            }
        }
    }
}
