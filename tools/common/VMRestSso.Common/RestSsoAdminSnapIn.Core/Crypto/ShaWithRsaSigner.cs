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
using System.Text;
using Org.BouncyCastle.Crypto.Parameters;
using Org.BouncyCastle.Security;
using Org.BouncyCastle.Math;
using Org.BouncyCastle.OpenSsl;
using Org.BouncyCastle.Crypto;
using System.IO;
using Org.BouncyCastle.X509;
using System.Security.Cryptography;
using Org.BouncyCastle.Asn1.X509;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Crypto
{
    public static class ShaWithRsaSigner
    {
        public static RsaPrivateCrtKeyParameters ReadPrivatePemKey(string filePath)
        {
            using (var reader = File.OpenText(filePath))
                return (RsaPrivateCrtKeyParameters)new PemReader(reader).ReadObject();
        }

		public static bool IsPrivateKeyValid(string filePath)
		{
			bool valid = true;

			try
			{
				var result = ReadPrivatePemKey (filePath);
				if(result == null)
					valid  = false;
			}
			catch(Exception exception) {
				valid = false;
			}
			return  valid;
		}

        public static X509Certificate ReadPublicPemCert(string filePath)
        {
            using (var reader = File.OpenText(filePath))
                return (X509Certificate)new PemReader(reader).ReadObject();
        }

        public static String Sign(String data, String privateKeyFilePath, string digestAlgorithm)
        {
            var keyPair = ReadPrivatePemKey(privateKeyFilePath);
            RsaKeyParameters key = (RsaKeyParameters)keyPair;
            var sig = SignerUtilities.GetSigner(digestAlgorithm + "withRSA");
            sig.Init(true, key);
            var bytes = Encoding.UTF8.GetBytes(data);
            sig.BlockUpdate(bytes, 0, bytes.Length);
            byte[] signature = sig.GenerateSignature();
            var signedString = Convert.ToBase64String(signature);
            return signedString;
        }

        public static bool Verify(String data, String expectedSignature, String publicKeyFilePath, string digestAlgorithm)
        {
            var cert = ReadPublicPemCert(publicKeyFilePath);
            RsaKeyParameters key = (RsaKeyParameters)cert.GetPublicKey();
            var signer = SignerUtilities.GetSigner(digestAlgorithm + "withRSA");
            signer.Init(false, key);
            var expectedSig = Convert.FromBase64String(expectedSignature);
            var msgBytes = Encoding.UTF8.GetBytes(data);
            signer.BlockUpdate(msgBytes, 0, msgBytes.Length);
            return signer.VerifySignature(expectedSig);
        }

        public static RSACryptoServiceProvider PrivatePemKeyToRSACryptoServiceProvider(string filePath)
        {
            var keyParams = ReadPrivatePemKey(filePath);            
            RSAParameters rsaParameters = DotNetUtilities.ToRSAParameters(keyParams);
            CspParameters cspParameters = new CspParameters();
            cspParameters.KeyContainerName = "MyKeyContainer";
            RSACryptoServiceProvider rsaKey = new RSACryptoServiceProvider(2048, cspParameters);
            rsaKey.ImportParameters(rsaParameters);
            return rsaKey;
        }

        public static string GetX500SubjectDN(System.Security.Cryptography.X509Certificates.X509Certificate2 certificate)
        {
            var bCert = DotNetUtilities.FromX509Certificate(certificate);
            return bCert.SubjectDN.ToString(true, X509Name.RFC2253Symbols);
        }

        public static string GetX500IssuerDN(System.Security.Cryptography.X509Certificates.X509Certificate2 certificate)
        {
            var bCert = DotNetUtilities.FromX509Certificate(certificate);
            return bCert.IssuerDN.ToString(true, X509Name.RFC2253Symbols);
        }    
    }
}
