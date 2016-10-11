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


using System.IO;
using System.Security.Cryptography;
using System.Security.Cryptography.Xml;
using System.Text;
using System.Xml;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers
{
    public static class SigningHelper
    {
        public static string SignXmlFile(string xml, RSA Key)
        {
            // Create a new XML document.
            var doc = new XmlDocument();

            // Format the document to ignore white spaces.
            doc.PreserveWhitespace = false;

            using (var textReader = new StringReader(xml))
            {
                doc.Load(new XmlTextReader(textReader));
            }

            // Create a SignedXml object.
            var signedXml = new SignedXmlWithId(doc);

            // Add the key to the SignedXml document. 
            signedXml.SigningKey = Key;

            // Specify a canonicalization method.
            signedXml.SignedInfo.CanonicalizationMethod = SignedXml.XmlDsigExcC14NTransformUrl;

            // Set the InclusiveNamespacesPrefixList property.        
            var canMethod = (XmlDsigExcC14NTransform)signedXml.SignedInfo.CanonicalizationMethodObject;
            var ref1 = new Reference("#Body52be6364-045f-1550-625d-b20b0390691e");
            var ref2 = new Reference("#Timestamp5257ab43-882c-4937-3835-6763e9a2d700");

            // Add an enveloped transformation to the reference.
            var env = new XmlDsigEnvelopedSignatureTransform();
            ref1.AddTransform(canMethod);
            ref2.AddTransform(canMethod);

            // Add the reference to the SignedXml object.
            signedXml.AddReference(ref1);
            signedXml.AddReference(ref2);

            string keyInfoStr = "<KeyInfo><wsse:SecurityTokenReference xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><wsse:Reference URI=\"#holderOfKeyCertificate\" ValueType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3\"/></wsse:SecurityTokenReference></KeyInfo>";
            var xd = new XmlDocument();
            xd.LoadXml(keyInfoStr);

            var ki = new KeyInfo();
            ki.LoadXml(xd.DocumentElement);
            signedXml.KeyInfo = ki;

            // Compute the signature.
            //signedXml.ComputeSignature(KeyedHashAlgorithm.Create("HMACSHA256"));
            signedXml.ComputeSignature();

            // Get the XML representation of the signature and save 
            // it to an XmlElement object.
            XmlElement xmlDigitalSignature = signedXml.GetXml();
            xmlDigitalSignature.SetAttribute("Id", "holderOfKeyProofSignature");

            var sb = new StringBuilder();
            using (var sw = new StringWriter(sb))
            {
                using (var writer = new XmlTextWriter(sw))
                {
                    xmlDigitalSignature.WriteTo(writer);
                }
            }
            return sb.ToString();
        }
    }
}
