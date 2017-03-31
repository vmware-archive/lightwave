using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Diffs;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Exceptions;
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Metadata;
using VmDirInterop.Schema.Utils;
using SchemaConnectionTest.Mocks;

namespace SchemaConnectionTest
{
    /// <summary>
    ///This is a test class for SchemaConnectionTest and is intended
    ///to contain all SchemaConnectionTest Unit Tests
    ///</summary>
    [TestClass()]
    public class SchemaConnectionTest
    {
        static SchemaConnection conn;
        static MockEntryFetcherFactory factory;
        static SchemaConnTestDataLoader testDataLoader;
        static string host1;
        static string host2;
        static string host3;
        static string upn;
        static string passwd;
        static string testDataFileNameFormat = @"..\..\..\TestData\{0}.xml";

        [ClassInitialize()]
        public static void MyClassInitialize(TestContext testContext)
        {
            host1 = "test-node-1";
            host2 = "test-node-2";
            host3 = "test-node-3";
            upn = "Administrator@vsphere.local";
            passwd = "123$%^";

            testDataLoader = new SchemaConnTestDataLoader();
            testDataLoader.LoadXML(string.Format(testDataFileNameFormat, host1));
            testDataLoader.LoadXML(string.Format(testDataFileNameFormat, host2));
            testDataLoader.LoadXML(string.Format(testDataFileNameFormat, host3));
            factory = new MockEntryFetcherFactory(testDataLoader);
            conn = new SchemaConnection(factory, host1, upn, passwd);
        }

        [TestMethod()]
        public void SchemaConnectionConstructorServerUnreachableTest()
        {
            try
            {
                SchemaConnection conn3 = new SchemaConnection(factory, host3, upn, passwd);
                Assert.Fail();
            }
            catch (SchemaConnectionException)
            {
                // pass
            }
            catch (Exception)
            {
                Assert.Fail();
            }
        }

        [TestMethod()]
        public void GetBaseServerNameTest()
        {
            Assert.AreEqual(host1, conn.GetBaseServerName());
        }

        [TestMethod()]
        public void GetAllServerStatusTest()
        {
            Dictionary<string, bool> expected = new Dictionary<string, bool>()
            {
                {"test-node-2", true},
                {"test-node-3", false},
            };

            Dictionary<string, bool> actual =
                (Dictionary<string, bool>)conn.GetAllServerStatus();

            CollectionAssert.AreEquivalent(expected, actual);
        }

        [TestMethod()]
        public void GetAllSchemaDefinitionDiffsTest()
        {
            TupleList<AttributeType, AttributeType> expectedAtDiff = new TupleList<AttributeType, AttributeType>()
            {
                new Tuple<AttributeType, AttributeType>(
                    new AttributeType("( 2.5.18.4 NAME 'modifiersName' DESC 'RFC4512: name of last modifier' SYNTAX 1.3.6.1.4.1.1466.115.121.1.12 NO-USER-MODIFICATION USAGE directoryOperation )"),
                    new AttributeType("( 2.5.18.4 NAME 'modifiersName' DESC 'RFC4512: name of last modifier' SYNTAX 1.3.6.1.4.1.1466.115.121.1.12 SINGLE-VALUE NO-USER-MODIFICATION USAGE directoryOperation )")),
                new Tuple<AttributeType, AttributeType>(
                    new AttributeType("( VMWare.DIR.attribute.999.11 NAME 'serverIdASDASDASD' DESC 'An ID used in SID allocation relative to this server' SYNTAX 1.3.6.1.4.1.1466.115.121.1.27 SINGLE-VALUE NO-USER-MODIFICATION USAGE directoryOperation )"),
                    null)
            };
            TupleList<ObjectClass, ObjectClass> expectedOcDiff = new TupleList<ObjectClass, ObjectClass>()
            {
                new Tuple<ObjectClass, ObjectClass>(
                    new ObjectClass("( VMWare.tagging.objectclass.3 NAME 'ASDFASDFASDF' DESC 'VMware CIS - tagging service tag' SUP top KIND-UNKNOWN MUST ( cn $ serverIdASDASDASD $ vmwTaggingTagName $ vmwTaggingTagVersion ) MAY ( vmwTaggingObjectState $ vmwTaggingTagDescription $ vmwTaggingTagUsedBy ) )"),
                    null),
                new Tuple<ObjectClass, ObjectClass>(
                    new ObjectClass("( VMWare.CA.objectclass.1 NAME 'vmwCertificationAuthority' SUP top STRUCTURAL AUX ( ASDFASDFASDF $ pkiCA ) MUST cn MAY cACertificateDN )"),
                    new ObjectClass("( VMWare.CA.objectclass.1 NAME 'vmwCertificationAuthority' SUP top STRUCTURAL AUX pkiCA MUST cn MAY cACertificateDN )")),
                new Tuple<ObjectClass, ObjectClass>(
                    new ObjectClass("( VMWare.tagging.objectclass.3 NAME 'vmwTaggingTagModel' DESC 'VMware CIS - tagging service tag' SUP top STRUCTURAL MUST ( cn $ vmwTaggingTagName $ vmwTaggingTagVersion ) MAY ( priorValue $ vmwTaggingAuthzUri $ vmwTaggingObjectState $ vmwTaggingTagDescription $ vmwTaggingTagUsedBy ) )"),
                    new ObjectClass("( VMWare.tagging.objectclass.3 NAME 'vmwTaggingTagModel' DESC 'VMware CIS - tagging service tag' SUP top STRUCTURAL MUST ( cn $ vmwTaggingTagName $ vmwTaggingTagVersion ) MAY ( vmwTaggingObjectState $ vmwTaggingTagDescription $ vmwTaggingTagUsedBy ) )"))
            };

            IDictionary<String, SchemaDefinitionDiff> diffs = conn.GetAllSchemaDefinitionDiffs();
            TupleList<AttributeType, AttributeType> actualAtDiff = diffs[host2].GetAttributeTypeDiff();
            TupleList<ObjectClass, ObjectClass> actualOcDiff = diffs[host2].GetObjectClassDiff();

            AssertAreAttributeTypeDiffEqual(expectedAtDiff, actualAtDiff);
            AssertAreObjectTypeDiffEqual(expectedOcDiff, actualOcDiff);
            Assert.IsNull(diffs[host3]);
        }

        private void AssertAreAttributeTypeDiffEqual(
            TupleList<AttributeType, AttributeType> expectedAtDiff,
            TupleList<AttributeType, AttributeType> actualAtDiff)
        {
            Assert.AreEqual(expectedAtDiff.Count, actualAtDiff.Count);
            for (int i = 0; i < expectedAtDiff.Count; i++)
            {
                AttributeType expectedAt1 = expectedAtDiff[i].Item1;
                AttributeType expectedAt2 = expectedAtDiff[i].Item2;
                AttributeType actualAt1 = actualAtDiff[i].Item1;
                AttributeType actualAt2 = actualAtDiff[i].Item2;

                if (expectedAt1 == null)
                {
                    Assert.IsNull(actualAt1);
                }
                else
                {
                    Assert.AreEqual(0, expectedAt1.FullCompareTo(actualAt1));
                }

                if (expectedAt2 == null)
                {
                    Assert.IsNull(actualAt2);
                }
                else
                {
                    Assert.AreEqual(0, expectedAt2.FullCompareTo(actualAt2));
                }
            }
        }

        private void AssertAreObjectTypeDiffEqual(
            TupleList<ObjectClass, ObjectClass> expectedOcDiff,
            TupleList<ObjectClass, ObjectClass> actualOcDiff)
        {
            Assert.AreEqual(expectedOcDiff.Count, actualOcDiff.Count);
            for (int i = 0; i < expectedOcDiff.Count; i++)
            {
                ObjectClass expectedOc1 = expectedOcDiff[i].Item1;
                ObjectClass expectedOc2 = expectedOcDiff[i].Item2;
                ObjectClass actualOc1 = actualOcDiff[i].Item1;
                ObjectClass actualOc2 = actualOcDiff[i].Item2;

                if (expectedOc1 == null)
                {
                    Assert.IsNull(actualOc1);
                }
                else
                {
                    Assert.AreEqual(0, expectedOc1.FullCompareTo(actualOc1));
                }

                if (expectedOc2 == null)
                {
                    Assert.IsNull(actualOc2);
                }
                else
                {
                    Assert.AreEqual(0, expectedOc2.FullCompareTo(actualOc2));
                }
            }
        }

        [TestMethod()]
        public void GetAllSchemaMetadataDiffsTest()
        {
            TupleList<SchemaEntry, SchemaEntry> expectedAtDiff = new TupleList<SchemaEntry, SchemaEntry>()
            {
                new Tuple<SchemaEntry, SchemaEntry>(
                    new SchemaEntry("modifiersName",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("isSingleValued:2153:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.017:2153"),
                            new AttributeMetadata("modifiersName:2153:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.017:2153"),
                            new AttributeMetadata("modifyTimestamp:2153:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.017:2153"),
                            new AttributeMetadata("uSNChanged:2153:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.017:2153"),
                        }),
                    new SchemaEntry("modifiersName",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("isSingleValued:865:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072911.732:865"),
                            new AttributeMetadata("modifyTimestamp:865:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072911.732:865"),
                            new AttributeMetadata("uSNChanged:865:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072911.732:865"),
                        })),
                new Tuple<SchemaEntry, SchemaEntry>(
                    new SchemaEntry("serverIdASDASDASD",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("attributeID:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("attributeSyntax:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("cn:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("createTimestamp:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("creatorsName:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("description:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("entryDN:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("isSingleValued:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("lDAPDisplayName:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("modifiersName:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("modifyTimestamp:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("objectClass:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("objectGUID:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("oMSyntax:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("schemaIDGUID:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("uSNChanged:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("uSNCreated:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                            new AttributeMetadata("vmwAttributeUsage:2152:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.016:2152"),
                        }),
                    null)
            };
            TupleList<SchemaEntry, SchemaEntry> expectedOcDiff = new TupleList<SchemaEntry, SchemaEntry>()
            {
                new Tuple<SchemaEntry, SchemaEntry>(
                    new SchemaEntry("ASDFASDFASDF",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("cn:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("createTimestamp:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("creatorsName:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("defaultObjectCategory:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("description:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("entryDN:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("governsID:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("modifiersName:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("modifyTimestamp:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("objectClass:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("objectClassCategory:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("objectGUID:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("schemaIDGUID:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("subClassOf:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("systemMayContain:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("systemMustContain:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("uSNChanged:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                            new AttributeMetadata("uSNCreated:2154:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.019:2154"),
                        }),
                    null),
                new Tuple<SchemaEntry, SchemaEntry>(
                    new SchemaEntry("vmwCertificationAuthority",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("auxiliaryClass:2156:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.021:2156"),
                            new AttributeMetadata("modifiersName:2156:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.021:2156"),
                            new AttributeMetadata("modifyTimestamp:2156:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.021:2156"),
                            new AttributeMetadata("uSNChanged:2156:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.021:2156"),
                        }),
                    new SchemaEntry("vmwCertificationAuthority",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("auxiliaryClass:1974:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072913.074:1974"),
                            new AttributeMetadata("modifyTimestamp:1974:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072913.074:1974"),
                            new AttributeMetadata("uSNChanged:1974:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072913.074:1974"),
                        })),
                new Tuple<SchemaEntry, SchemaEntry>(
                    new SchemaEntry("vmwTaggingTagModel",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("modifiersName:2155:1:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.020:2155"),
                            new AttributeMetadata("modifyTimestamp:2155:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.020:2155"),
                            new AttributeMetadata("systemMayContain:2155:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.020:2155"),
                            new AttributeMetadata("uSNChanged:2155:2:d4c1acfa-2a73-4311-abdc-23fa1741361d:20160326073355.020:2155"),
                        }),
                    new SchemaEntry("vmwTaggingTagModel",
                        new List<AttributeMetadata>(){
                            new AttributeMetadata("modifyTimestamp:2008:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072913.111:2008"),
                            new AttributeMetadata("systemMayContain:2008:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072913.111:2008"),
                            new AttributeMetadata("uSNChanged:2008:1:42e80f12-3bf6-4812-887a-6524e2a827b8:20160326072913.111:2008"),
                        }))
            };

            IDictionary<String, SchemaMetadataDiff> diffs = conn.GetAllSchemaMetadataDiffs();
            TupleList<SchemaEntry, SchemaEntry> actualAtDiff = diffs[host2].GetAttributeTypeDiff();
            TupleList<SchemaEntry, SchemaEntry> actualOcDiff = diffs[host2].GetObjectClassDiff();

            AssertAreSchemaEntriesEqual(expectedAtDiff, actualAtDiff);
            AssertAreSchemaEntriesEqual(expectedOcDiff, actualOcDiff);
            Assert.IsNull(diffs[host3]);
        }

        private void AssertAreSchemaEntriesEqual(
            TupleList<SchemaEntry, SchemaEntry> expectedDiff,
            TupleList<SchemaEntry, SchemaEntry> actualDiff
            )
        {
            Assert.AreEqual(expectedDiff.Count, actualDiff.Count);
            for (int i = 0; i < expectedDiff.Count; i++)
            {
                SchemaEntry expected1 = expectedDiff[i].Item1;
                SchemaEntry expected2 = expectedDiff[i].Item2;
                SchemaEntry actual1 = actualDiff[i].Item1;
                SchemaEntry actual2 = actualDiff[i].Item2;

                if (expected1 != null && expected2 != null)
                {
                    TupleList<AttributeMetadata, AttributeMetadata> expectedMdDiff =
                        expected1.GetMetadataList().GetDiff(expected2.GetMetadataList());
                    TupleList<AttributeMetadata, AttributeMetadata> actualMdDiff =
                        actual1.GetMetadataList().GetDiff(actual2.GetMetadataList());

                    AssertAreMetadataEqual(expectedMdDiff, actualMdDiff);
                }
                else if (expected1 == null)
                {
                    Assert.IsNull(actual1);
                }
                else if (expected2 == null)
                {
                    Assert.IsNull(actual2);
                }
                else
                {
                    Assert.Fail();  // they both can't be null
                }
            }
        }

        private void AssertAreMetadataEqual(
            TupleList<AttributeMetadata, AttributeMetadata> expectedMdDiff,
            TupleList<AttributeMetadata, AttributeMetadata> actualMdDiff)
        {
            Assert.AreEqual(expectedMdDiff.Count, actualMdDiff.Count);
            for (int i = 0; i < expectedMdDiff.Count; i++)
            {
                AttributeMetadata expectedMd1 = expectedMdDiff[i].Item1;
                AttributeMetadata expectedMd2 = expectedMdDiff[i].Item2;
                AttributeMetadata actualMd1 = actualMdDiff[i].Item1;
                AttributeMetadata actualMd2 = actualMdDiff[i].Item2;

                if (expectedMd1 == null)
                {
                    Assert.IsNull(actualMd1);
                }
                else
                {
                    Assert.AreEqual(0, expectedMd1.FullCompareTo(actualMd1));
                }

                if (expectedMd2 == null)
                {
                    Assert.IsNull(actualMd2);
                }
                else
                {
                    Assert.AreEqual(0, expectedMd2.FullCompareTo(actualMd2));
                }
            }
        }

        [TestMethod()]
        public void RefreshSchemaConnectionTest()
        {
            SchemaConnTestData testData = testDataLoader.GetTestData(host3);
            testData.Reachable = true;
            conn.RefreshSchemaConnection(upn, passwd);

            Dictionary<string, bool> expected = new Dictionary<string, bool>()
            {
                {"test-node-2", true},
                {"test-node-3", true},
            };

            Dictionary<string, bool> actual =
                (Dictionary<string, bool>)conn.GetAllServerStatus();

            CollectionAssert.AreEquivalent(expected, actual);

            testData.Reachable = false;
            conn.RefreshSchemaConnection(upn, passwd);

            expected = new Dictionary<string, bool>()
            {
                {"test-node-2", true},
                {"test-node-3", false},
            };

            actual = (Dictionary<string, bool>)conn.GetAllServerStatus();

            CollectionAssert.AreEquivalent(expected, actual);
        }
    }
}
