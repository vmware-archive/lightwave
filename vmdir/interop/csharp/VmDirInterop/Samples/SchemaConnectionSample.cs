using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Diffs;
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Utils;
using VmDirInterop.Schema.Metadata;
using VmDirInterop.Schema.Entries;
//using SchemaConnectionTest.Mocks;

namespace Samples
{
    class SchemaConnetionSample
    {
        public static void Run()
        {
            String host = "your-test-node";
            String upn = "Administrator@vsphere.local";
            String passwd = "Admin!23";

            SchemaConnection conn = new SchemaConnection(host, upn, passwd);

            //String host1 = "test-node-1";
            //String host2 = "test-node-2";
            //String host3 = "test-node-3";
            //String upn = "Administrator@vsphere.local";
            //String passwd = "Admin!23";
            //String testDataFileNameFormat = @"..\..\..\TestData\{0}.xml";

            //SchemaConnTestDataLoader testDataLoader = new SchemaConnTestDataLoader();
            //testDataLoader.LoadXML(string.Format(testDataFileNameFormat, host1));
            //testDataLoader.LoadXML(string.Format(testDataFileNameFormat, host2));
            //testDataLoader.LoadXML(string.Format(testDataFileNameFormat, host3));
            //MockEntryFetcherFactory factory = new MockEntryFetcherFactory(testDataLoader);
            //SchemaConnection conn = new SchemaConnection(factory, host1, upn, passwd);

            listNodeAvailability(conn);
            listSchemaDefinitionDiff(conn);
            listSchemaMetadataDiff(conn);

            return;
        }

        private static void listNodeAvailability(ISchemaConnection conn)
        {
            Console.WriteLine("SERVER AVAILABILLITY");
            Console.WriteLine("---------");
            Console.WriteLine("Base: {0}", conn.GetBaseServerName());
            foreach (KeyValuePair<String, Boolean> p in conn.GetAllServerStatus())
            {
                Console.WriteLine("Key = {0}, Value = {1}", p.Key, p.Value);
            }
            Console.WriteLine();
            Console.WriteLine();
        }

        private static void listSchemaDefinitionDiff(ISchemaConnection conn)
        {
            Console.WriteLine("SCHEMA DEFINITION DIFF");
            Console.WriteLine("---------");

            String baseServer = conn.GetBaseServerName();
            IDictionary<String, SchemaDefinitionDiff> diffs = conn.GetAllSchemaDefinitionDiffs();
            foreach (KeyValuePair<String, SchemaDefinitionDiff> p in diffs)
            {
                String server = p.Key;
                SchemaDefinitionDiff diff = p.Value;

                Console.WriteLine("Schema comparsion between {0} and {1}", baseServer, server);
                if (diff != null)
                {
                    foreach (Tuple<AttributeType, AttributeType> t in diff.GetAttributeTypeDiff())
                    {
                        Console.WriteLine("\tBASE: {0}", t.Item1);
                        Console.WriteLine("\tOTHE: {0}", t.Item2);
                        Console.WriteLine("\t-----");
                    }
                    foreach (Tuple<ObjectClass, ObjectClass> t in diff.GetObjectClassDiff())
                    {
                        Console.WriteLine("\tBASE: {0}", t.Item1);
                        Console.WriteLine("\tOTHE: {0}", t.Item2);
                        Console.WriteLine("\t-----");
                    }
                }
                else
                {
                    Console.WriteLine("\tServer not reachable {0}", server);
                }
                Console.WriteLine();
            }
            Console.WriteLine();
        }

        private static void listSchemaMetadataDiff(ISchemaConnection conn)
        {
            Console.WriteLine("SCHEMA METADATA DIFF");
            Console.WriteLine("---------");

            String baseServer = conn.GetBaseServerName();
            IDictionary<String, SchemaMetadataDiff> diffs = conn.GetAllSchemaMetadataDiffs();
            foreach (KeyValuePair<String, SchemaMetadataDiff> p in diffs)
            {
                String server = p.Key;
                SchemaMetadataDiff diff = p.Value;

                Console.WriteLine("Metadata comparsion between {0} and {1}", baseServer, server);
                if (diff != null)
                {
                    foreach (Tuple<SchemaEntry, SchemaEntry> t in diff.GetAttributeTypeDiff())
                    {
                        listSchemaMetadataDiffBreakdown(t.Item1, t.Item2);
                    }
                    foreach (Tuple<SchemaEntry, SchemaEntry> t in diff.GetObjectClassDiff())
                    {
                        listSchemaMetadataDiffBreakdown(t.Item1, t.Item2);
                    }
                }
                else
                {
                    Console.WriteLine("\tServer not reachable {0}", server);
                }
                Console.WriteLine();
            }
            Console.WriteLine();
        }

        private static void listSchemaMetadataDiffBreakdown(SchemaEntry e1, SchemaEntry e2)
        {
            SchemaComparableList<AttributeMetadata> mdList1 = null;
            SchemaComparableList<AttributeMetadata> mdList2 = null;

            if (e1 != null && e2 != null)
            {
                mdList1 = e1.GetMetadataList();
                mdList2 = e2.GetMetadataList();

                TupleList<AttributeMetadata, AttributeMetadata> diff = mdList1.GetDiff(mdList2);

                Console.WriteLine("\tBASE: {0}", e1.defName);
                foreach (Tuple<AttributeMetadata, AttributeMetadata> t in diff)
                {
                    Console.WriteLine("\t\t{0}", t.Item1);
                }

                Console.WriteLine("\tOTHE: {0}", e2.defName);
                foreach (Tuple<AttributeMetadata, AttributeMetadata> t in diff)
                {
                    Console.WriteLine("\t\t{0}", t.Item2);
                }
            }
            else if (e1 != null)
            {
                mdList1 = e1.GetMetadataList();

                Console.WriteLine("\tBASE: {0}", e1.defName);
                foreach (AttributeMetadata md in mdList1)
                {
                    Console.WriteLine("\t\t{0}", md);
                }
                Console.WriteLine("\tOTHE:");
            }
            else
            {
                mdList2 = e2.GetMetadataList();

                Console.WriteLine("\tBASE:");
                Console.WriteLine("\tOTHE: {0}", e2.defName);
                foreach (AttributeMetadata md in mdList2)
                {
                    Console.WriteLine("\t\t{0}", md);
                }
            }

            Console.WriteLine("\t-----");
        }
    }
}
