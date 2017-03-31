using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Interfaces;

namespace SchemaConnectionTest.Mocks
{
    public class MockEntryFetcher : IEntryFetcher
    {
        private SchemaConnTestData testData;

        public MockEntryFetcher(SchemaConnTestData testData)
        {
            this.testData = testData;
        }

        public DseRootEntry GetDseRootEntry()
        {
            return testData.DseRootEntry;
        }

        public ServerEntry GetServerEntry(String dn)
        {
            return testData.ServerEntries[dn];
        }

        public SubSchemaSubEntry GetSubSchemaSubEntry()
        {
            return testData.SubSchemaSubEntry;
        }

        public IList<ServerEntry> GetServerEntries()
        {
            return new List<ServerEntry>(testData.ServerEntries.Values);
        }

        public SchemaComparableList<SchemaEntry> GetAttributeSchemaEntries()
        {
            return testData.AttributeSchemaEntries;
        }

        public SchemaComparableList<SchemaEntry> GetClassSchemaEntries()
        {
            return testData.ClassSchemaEntries;
        }
    }
}
