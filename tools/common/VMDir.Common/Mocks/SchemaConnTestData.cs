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
using VmDirInterop.Schema;
using VmDirInterop.Schema.Entries;

//TODO - This file is for testing purposes to provide a mock server with test data. Will be removed.
namespace SchemaConnectionTest.Mocks
{
    public class SchemaConnTestData
    {
        public String HostName { get; set; }

        public Boolean Reachable { get; set; }

        public DseRootEntry DseRootEntry { get; set; }

        public IDictionary<String, ServerEntry> ServerEntries { get; set; }

        public SubSchemaSubEntry SubSchemaSubEntry { get; set; }

        public SchemaComparableList<SchemaEntry> AttributeSchemaEntries { get; set; }

        public SchemaComparableList<SchemaEntry> ClassSchemaEntries { get; set; }
    }
}
