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

/*
 * DNS Constants using in the UI
 *
 * @author Sumalatha Abhishek
 */

namespace VMDNS.Common
{
    public  static class VMDNSConstants
    {
        /*dialog result codes */
        public const int DIALOGOK = 1;
        public const int DIALOGCANCEL = -1;

        /* Welcome Screen Constants */
        public const string WELCOME_TITLE_DESCRIPTION = "Welcome to Lightwave DNS.";
        public const string WELCOME_DESCRIPTION1 = "Administer Zones";
        public const string WELCOME_DESCRIPTION2 = "Administer DNS Records";
        public const string WELCOME_DESCRIPTION3 = "Configure Forwarders";

        /* Application Constants */
        public const string VMDNS_APPNAME = "Lightwave DNS";
        public const string VMDNS_DATA_FILENAME = "VMDNSData.xml";
        public const string VMDNS_SNAPIN = "VMware DNS SnapIn";
        public const string VMDNS_DESCRIPTION = "VMware Domain Name Service SnapIn";

        public const int DNSSERVER_TIMEOUT = 30000;

        /* Zone Constants */
        public const string FORWARDZONE = "Forward Zones";
        public const string REVERSEZONE = "Reverse Zones";
        public const string ZONES = "Zones";
        public const string ZONE_NAME = "ZoneName";
        public const string DNS_NAME = "DNSName";
        public const string ADMIN_EMAIL = "AdminEmail";
        public const string REFRESH_INTERVAL = "RefreshInterval";
        public const string RETRY_INTERVAL = "RetryInterval";

        /* Record  Tableview Constants */
        public const string RECORD_NAME = "RecordName";
        public const string RECORD_TYPE = "RecordType";
        public const string RECORD_CLASS = "RecordClass";

        public const string RECORD_A = "A";
        public const string RECORD_AAAA = "AAAA";
        public const string RECORD_NS = "NS";
        public const string RECORD_CNAME = "CNAME";
        public const string RECORD_PTR = "PTR";
        public const string RECORD_SRV = "SRV";
        public const string RECORD_SOA = "SOA";

        /* Menu Constants */
        public const string ZONE_PROPERTIES = "Zone Properties";
        public const string RECORD_PROPERTIES = "Record Properties";
        public const string ZONE_DELETE = "Delete Zone";
        public const string RECORD_DELETE = "Delete Record";
        public const string RECORD_ADD = "Add New Record";
        public const string ZONE_ADD_FORWARD = "Add Forward Zone";
        public const string ZONE_ADD_REVERSE = "Add Reverse Zone";
        public const string SERVER_CONFIG = "Server Configuration";
        public const string REFRESH = "Refresh";
        public const string EDIT = "Edit";
        public const string UPDATE = "Update";

        /* Dialog Messages */
        public const string SUCCESS_DELETE_RECORD = "Successfully Deleted Record";
        public const string SUCCESS_ADD_RECORD = "Successfully Added Record";
        public const string SUCCESS_DELETE_ZONE = "Successfully Deleted Zone";
        public const string SELECT_ZONE = "Please select a zone from left pane to start search records";
        public const string IP_VALIDATE = "Please enter valid IP address";
        public const string SUCCESS_UPDATE_ZONE = "Successfully Update Zone";

    }
}

