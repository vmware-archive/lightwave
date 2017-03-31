using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Utils;

namespace SchemaConnectionTest
{
    /// <summary>
    ///This is a test class for SchemaComparableListTest and is intended
    ///to contain all SchemaComparableListTest Unit Tests
    ///</summary>
    [TestClass()]
    public class SchemaComparableListTest
    {
        static IList<TestSchemaComparable> testList1 = new List<TestSchemaComparable>
        {
            new TestSchemaComparable("A", "AAA"),
            new TestSchemaComparable("B", "BBB"),
            new TestSchemaComparable("C", "CCC"),
            new TestSchemaComparable("D", "DDD"),
            new TestSchemaComparable("F", "FFF"),
            new TestSchemaComparable("G", "GGG")
        };

        static IList<TestSchemaComparable> testList2 = new List<TestSchemaComparable>
        {
            new TestSchemaComparable("B", "BBB"),
            new TestSchemaComparable("E", "EEE"),
            new TestSchemaComparable("D", "DD"),
            new TestSchemaComparable("C", "CC"),
            new TestSchemaComparable("A", "AAA")
        };

        [TestMethod()]
        public void SchemaComparableListConstructorTest()
        {
            SchemaComparableList<TestSchemaComparable> scList1 =
                new SchemaComparableList<TestSchemaComparable>(testList1);

            Assert.AreEqual(testList1[0], scList1[0]);
            Assert.AreEqual(testList1[1], scList1[1]);
            Assert.AreEqual(testList1[2], scList1[2]);
            Assert.AreEqual(testList1[3], scList1[3]);
            Assert.AreEqual(testList1[4], scList1[4]);
            Assert.AreEqual(testList1[5], scList1[5]);

            SchemaComparableList<TestSchemaComparable> scList2 =
                new SchemaComparableList<TestSchemaComparable>(testList2);

            // Elements are always sorted
            Assert.AreEqual(testList2[4], scList2[0]);
            Assert.AreEqual(testList2[0], scList2[1]);
            Assert.AreEqual(testList2[3], scList2[2]);
            Assert.AreEqual(testList2[2], scList2[3]);
            Assert.AreEqual(testList2[1], scList2[4]);
        }

        [TestMethod()]
        public void GetDiffTest()
        {
            SchemaComparableList<TestSchemaComparable> scList1 =
                new SchemaComparableList<TestSchemaComparable>(testList1);

            SchemaComparableList<TestSchemaComparable> scList2 =
                new SchemaComparableList<TestSchemaComparable>(testList2);

            TupleList<TestSchemaComparable, TestSchemaComparable> diff = scList1.GetDiff(scList2);

            Assert.AreEqual("C-CCC", diff[0].item1.full);
            Assert.AreEqual("C-CC", diff[0].item2.full);

            Assert.AreEqual("D-DDD", diff[1].item1.full);
            Assert.AreEqual("D-DD", diff[1].item2.full);

            Assert.IsNull(diff[2].item1);
            Assert.AreEqual("E-EEE", diff[2].item2.full);

            Assert.AreEqual("F-FFF", diff[3].item1.full);
            Assert.IsNull(diff[3].item2);

            Assert.AreEqual("G-GGG", diff[4].item1.full);
            Assert.IsNull(diff[4].item2);
        }

        [TestMethod()]
        public void GetDiffNullArgumentTest()
        {
            SchemaComparableList<TestSchemaComparable> scList1 =
                new SchemaComparableList<TestSchemaComparable>(testList1);

            try
            {
                scList1.GetDiff(null);
                Assert.Fail();
            }
            catch (ArgumentNullException)
            {
                // pass
            }
            catch (Exception)
            {
                Assert.Fail();
            }
        }
    }

    class TestSchemaComparable : ISchemaComparable<TestSchemaComparable>
    {
        public string name { get; private set; }
        public string etc { get; private set; }
        public string full { get; private set; }

        public TestSchemaComparable(string name, string etc)
        {
            this.name = name;
            this.etc = etc;
            this.full = name + "-" + etc;
        }

        public int CompareTo(TestSchemaComparable other)
        {
            return NameCompareTo(other);
        }

        public int NameCompareTo(TestSchemaComparable other)
        {
            return string.Compare(this.name, other.name);
        }

        public int FullCompareTo(TestSchemaComparable other)
        {
            return string.Compare(this.full, other.full);
        }
    }
}
