using Microsoft.VisualStudio.TestTools.UnitTesting;
using VmDirInterop.Schema.Definitions;

namespace SchemaConnectionTest
{
    /// <summary>
    ///This is a test class for ObjectClassTest and is intended
    ///to contain all ObjectClassTest Unit Tests
    ///</summary>
    [TestClass()]
    public class ObjectClassTest
    {
        static string ocStr = "( 1.2.840.113556.1.5.24 NAME 'remoteMailRecipient' SUP top STRUCTURAL MAY remoteSource )";
        static string crStr = "( 1.2.840.113556.1.5.24 NAME 'remoteMailRecipient' AUX ( bootableDevice $ domainRelatedObject ) MUST ( cn $ info $ labeledURI ) MAY ( secretary $ showInAddressBook $ telephoneNumber $ userCertificate ) )";
        static string badOcStr = "( 1.2.840.11 NAME 'remoteMailRecipient' STRUCTURAL MAY telephoneNumber )";

        /// <summary>
        ///A test for ObjectClass Constructor
        ///</summary>
        [TestMethod()]
        public void ObjectClassConstructorTest()
        {
            string oid = "1.2.840.113556.1.5.24";
            string name = "remoteMailRecipient";
            string other = "SUP top STRUCTURAL";
            string must = string.Empty;
            string may = "remoteSource";

            ObjectClass oc = new ObjectClass(ocStr);

            Assert.AreEqual(oid, oc.oid);
            Assert.AreEqual(name, oc.name);
            Assert.AreEqual(other, oc.other);
            Assert.AreEqual(must, oc.must);
            Assert.AreEqual(may, oc.may);
            Assert.AreEqual(ocStr, oc.ToString());
        }

        /// <summary>
        ///A test for MergeContentRule
        ///</summary>
        [TestMethod()]
        public void MergeContentRuleTest()
        {
            string oid = "1.2.840.113556.1.5.24";
            string name = "remoteMailRecipient";
            string other = "SUP top STRUCTURAL";
            string aux = "( bootableDevice $ domainRelatedObject )";
            string must = "( cn $ info $ labeledURI )";
            string may = "( remoteSource $ secretary $ showInAddressBook $ telephoneNumber $ userCertificate )";
            string newOcStr = "( 1.2.840.113556.1.5.24 NAME 'remoteMailRecipient' SUP top STRUCTURAL AUX ( bootableDevice $ domainRelatedObject ) MUST ( cn $ info $ labeledURI ) MAY ( remoteSource $ secretary $ showInAddressBook $ telephoneNumber $ userCertificate ) )";

            ObjectClass oc = new ObjectClass(ocStr);
            ContentRule cr = new ContentRule(crStr);
            oc.MergeContentRule(cr);

            Assert.AreEqual(oid, oc.oid);
            Assert.AreEqual(name, oc.name);
            Assert.AreEqual(other, oc.other);
            Assert.AreEqual(aux, oc.aux);
            Assert.AreEqual(must, oc.must);
            Assert.AreEqual(may, oc.may);
            Assert.AreEqual(newOcStr, oc.ToString());
        }

        /// <summary>
        ///A test for NameCompareTo
        ///</summary>
        [TestMethod()]
        public void NameCompareToTest()
        {
            ObjectClass oc = new ObjectClass(ocStr);
            ObjectClass badOc = new ObjectClass(badOcStr);

            Assert.AreEqual(0, oc.NameCompareTo(badOc));
        }

        /// <summary>
        ///A test for FullCompareTo
        ///</summary>
        [TestMethod()]
        public void FullCompareToTest()
        {
            ObjectClass oc1 = new ObjectClass(ocStr);
            ObjectClass oc2 = new ObjectClass(ocStr);
            ObjectClass badOc = new ObjectClass(badOcStr);

            Assert.AreEqual(0, oc1.FullCompareTo(oc2));
            Assert.AreNotEqual(0, oc1.FullCompareTo(badOc));
        }

        /// <summary>
        ///A test for CompareTo
        ///</summary>
        [TestMethod()]
        public void CompareToTest()
        {
            ObjectClass oc = new ObjectClass(ocStr);
            ObjectClass badOc = new ObjectClass(badOcStr);

            Assert.AreEqual(oc.NameCompareTo(badOc), oc.CompareTo(badOc));
        }
    }
}
