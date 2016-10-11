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
using VMDNS.Client;
using System.Net;

namespace VMDNS.Common
{
    public static class VMDNSUtilityService
    {
        //Move thes below 2 utility funtions to a class and use dictionaries instead of switch to fetch type.
        public static RecordType GetRecordType(string recordName)
        {
            RecordType type = new RecordType();
            switch (recordName)
            {
                case "A":
                    type = RecordType.VMDNS_RR_TYPE_A;
                    break;
                case "AAAA":
                    type = RecordType.VMDNS_RR_TYPE_AAAA;
                    break;
                case "NS":
                    type = RecordType.VMDNS_RR_TYPE_NS;
                    break;
                case "PTR":
                    type = RecordType.VMDNS_RR_TYPE_PTR;
                    break;
                case "CNAME":
                    type = RecordType.VMDNS_RR_TYPE_CNAME;
                    break;
                case "SRV":
                    type = RecordType.VMDNS_RR_TYPE_SRV;
                    break;
                case "SOA":
                    type = RecordType.VMDNS_RR_TYPE_SOA;
                    break;
            }
            return type;
        }

        public static string GetRecordNameFromType(RecordType recordType)
        {
            string value = string.Empty;
            switch (recordType)
            {
                case RecordType.VMDNS_RR_TYPE_A:
                    value = VMDNSConstants.RECORD_A;
                    break;
                case RecordType.VMDNS_RR_TYPE_AAAA:
                    value = VMDNSConstants.RECORD_AAAA;
                    break;
                case RecordType.VMDNS_RR_TYPE_NS:
                    value = VMDNSConstants.RECORD_NS;
                    break;
                case RecordType.VMDNS_RR_TYPE_PTR:
                    value = VMDNSConstants.RECORD_PTR;
                    break;
                case RecordType.VMDNS_RR_TYPE_CNAME:
                    value = VMDNSConstants.RECORD_CNAME;
                    break;
                case RecordType.VMDNS_RR_TYPE_SRV:
                    value = VMDNSConstants.RECORD_SRV;
                    break;
                case RecordType.VMDNS_RR_TYPE_SOA:
                    value = VMDNSConstants.RECORD_SOA;
                    break;
            }
            return value;
        }

        public static bool IsValidIPAddress(string inputIpAddress)
        {
            IPAddress address;

            bool val = (!string.IsNullOrWhiteSpace(inputIpAddress) && IPAddress.TryParse(inputIpAddress, out address));
            return val;
        }
    }
}

