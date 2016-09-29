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
using System.IO;
using System.Security.Cryptography;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers
{
    public class PrivateKeyHelper
    {
        public static string ExtractBase64EncodedPayload(string keyText)
        {
            try
            {
                string BEGIN = "-----BEGIN RSA PRIVATE KEY-----";
                string END = "-----END RSA PRIVATE KEY-----";

                string BEGIN2 = "-----BEGIN PRIVATE KEY-----";
                string END2 = "-----END PRIVATE KEY-----";

                int startIndex = keyText.IndexOf(BEGIN);
                if (startIndex == -1)
                {
                    BEGIN = BEGIN2;
                    startIndex = keyText.IndexOf(BEGIN);
                }
                if (startIndex == -1)
                    startIndex = 0;
                else
                    startIndex += BEGIN.Length;
                int endIndex = keyText.IndexOf(END);
                if (endIndex == -1)
                    endIndex = keyText.IndexOf(END2);
                if (endIndex == -1)
                    endIndex = keyText.Length;
                return keyText.Substring(startIndex, endIndex - startIndex).Trim();
            }
            catch (Exception)
            {
                throw new Exception("Could not read private key file");
            }
        }

        //------- Parses binary ans.1 RSA private key; returns RSACryptoServiceProvider  ---
        public static RSACryptoServiceProvider DecodeRSAPrivateKey(byte[] privkey)
        {
            byte[] MODULUS, E, D, P, Q, DP, DQ, IQ;

            // ---------  Set up stream to decode the asn.1 encoded RSA private key  ------
            MemoryStream mem = new MemoryStream(privkey);
            BinaryReader binr = new BinaryReader(mem);    //wrap Memory Stream with BinaryReader for easy reading
            byte bt = 0;
            ushort twobytes = 0;
            int elems = 0;
            try
            {
                twobytes = binr.ReadUInt16();
                if (twobytes == 0x8130) //data read as little endian order (actual data order for Sequence is 30 81)
                    binr.ReadByte();        //advance 1 byte
                else if (twobytes == 0x8230)
                    binr.ReadInt16();       //advance 2 bytes
                else
                    return null;

                twobytes = binr.ReadUInt16();
                if (twobytes != 0x0102) //version number
                    return null;
                bt = binr.ReadByte();
                if (bt != 0x00)
                    return null;


                //------  all private key components are Integer sequences ----
                elems = SystemHelper.GetIntegerSize(binr);
                MODULUS = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                E = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                D = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                P = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                Q = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                DP = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                DQ = binr.ReadBytes(elems);

                elems = SystemHelper.GetIntegerSize(binr);
                IQ = binr.ReadBytes(elems);


                // ------- create RSACryptoServiceProvider instance and initialize with public key -----
                RSACryptoServiceProvider RSA = new RSACryptoServiceProvider();
                RSAParameters RSAparams = new RSAParameters();
                RSAparams.Modulus = MODULUS;
                RSAparams.Exponent = E;
                RSAparams.D = D;
                RSAparams.P = P;
                RSAparams.Q = Q;
                RSAparams.DP = DP;
                RSAparams.DQ = DQ;
                RSAparams.InverseQ = IQ;
                RSA.ImportParameters(RSAparams);
                return RSA;
            }
            catch (Exception)
            {
                return null;
            }
            finally
            {
                binr.Close();
            }
        }
    }
}
