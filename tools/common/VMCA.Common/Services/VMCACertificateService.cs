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
using System.Security.Cryptography.X509Certificates;
using VMCA.Client;
using VMCASnapIn.DTO;
using System.IO;

//using System.Web.UI;


namespace VMCASnapIn.Services
{
    public static class VMCACertificateService
    {
        public static void Validate (VMCAServerDTO serverDTO, string cert)
        {
            try {
                VMCACertificate.Validate (cert);
            } catch (Exception e) {
                throw e;
            }
        }

        public static void RevokeCertificate (X509Certificate2 cert, VMCAServerDTO dto)
        {
            try {
                var vmcaCert = new VMCACertificate (dto.VMCAClient, cert);
                vmcaCert.Revoke ();
            } catch (Exception e) {
                throw e;
            }
        }

        public static string GetVersion (VMCAServerDTO dto)
        {
            string version = "";
            try {
                version = dto.VMCAClient.GetServerVersion ();
            } catch (Exception e) {
                throw e;
            }
            return version;
        }

        public static string GetHTMLStringFromX509Certificate2 ()
        {
            //string html =  string.Format (@"<html><title></title><head></head><body> {0} </body></html>", "Hello");
            return "Hello";

        }
    }
}
