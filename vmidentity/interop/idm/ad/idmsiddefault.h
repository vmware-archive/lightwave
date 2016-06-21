/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

/*
 * Module Name:
 *
 *        idmsiddefault.h
 *
 * Abstract:
 *
 *        Identity Manager - Active Directory Integration
 *
 *        SID default values
 *
 * Authors: Adam Bernstein (abernstein@vmware.com)
 *
 */

/*
 * Reference: http://msdn.microsoft.com/en-us/library/cc980032.aspx
 *            http://support.microsoft.com/kb/243330
 *            http://support.microsoft.com/kb/2830145
 */


/*
 * Special SIDS:
 * S-1-5-5-X-Y
 * S-1-5-21-X-Y-Z-498
 * S-1-5-80-0
 */
static
IDM_WELL_KNOWN_SID SpecialSidPrefixes[] =
{
    { "S-1-5-5-X-Y",
      "Logon Session" },

    { "S-1-5-21-X-Y-Z-498",
      "Enterprise Read-only Domain Controllers" },
    { "S-1-5-21-X-Y-Z-500",
      "Administrator" },
    { "S-1-5-21-X-Y-Z-501",
      "Guest" },
    { "S-1-5-21-X-Y-Z-502",
      "KRBTGT" },
    { "S-1-5-21-X-Y-Z-512",
      "Domain Admins" },
    { "S-1-5-21-X-Y-Z-513",
      "Domain Users" },
    { "S-1-5-21-X-Y-Z-514",
      "Domain Guests" },
    { "S-1-5-21-X-Y-Z-515",
      "Domain Computers" },
    { "S-1-5-21-X-Y-Z-516",
      "Domain Controllers" },
    { "S-1-5-21-X-Y-Z-517",
      "Cert Publishers" },
    { "S-1-5-21-X-Y-Z-518",
      "Schema Admins" },
    { "S-1-5-21-X-Y-Z-519",
      "Enterprise Admins" },
    { "S-1-5-21-X-Y-Z-520",
      "Group Policy Creator Owners" },
    { "S-1-5-21-X-Y-Z-521",
      "Read-only Domain Controllers" },
    { "S-1-5-21-X-Y-Z-522",
      "Cloneable Domain Controllers" },
    { "S-1-5-21-X-Y-Z-553",
      "RAS and IAS Servers" },
    { "S-1-5-21-X-Y-Z-571",
      "Allowed RODC Password Replication Group" },
    { "S-1-5-21-X-Y-Z-572",
      "Denied RODC Password Replication Group" },

    { "S-1-5-80-0",
      "All Services" },
};


static
IDM_WELL_KNOWN_SID WellKnownSids[] = 
{
    { "S-1-0",
      "Null Authority" },
    { "S-1-0-0",
      "Nobody" },
    { "S-1-1",
      "World Authority" },
    { "S-1-1-0",
      "Everyone" },
    { "S-1-2",
      "Local Authority" },
    { "S-1-2-0",
      "Local" },
    { "S-1-2-1",
      "Console Logon" },
    { "S-1-3",
      "Creator Authority" },
    { "S-1-3-0",
      "Creator Owner" },
    { "S-1-3-1",
      "Creator Group" },
    { "S-1-3-2",
      "Creator Owner Server" },
    { "S-1-3-3",
      "Creator Group Server" },
#if 0
      "Owner Rights"
#endif
    { "S-1-4",
      "Non-unique Authority" },
    { "S-1-5",
      "NT Authority" },
    { "S-1-5-1",
      "Dialup" },
    { "S-1-5-2",
      "Network" },
    { "S-1-5-3",
      "Batch" },
    { "S-1-5-4",
      "Interactive" },
/*
 * A logon session. The X and Y values for these SIDs are different for each
 * logon session and are recycled when the operating system is restarted.
 */
#if 1
    { "S-1-5-5",
      "Logon Session" },
#else
    { "S-1-5-5-X-Y",
      "Logon Session" },
#endif
    { "S-1-5-6",
      "Service" },
    { "S-1-5-7",
      "Anonymous" },
    { "S-1-5-8",
      "Proxy" },
    { "S-1-5-9",
      "Enterprise Domain Controllers" },
    { "S-1-5-10",
      "Principal Self" },
    { "S-1-5-11",
      "Authenticated Users" },
    { "S-1-5-12",
      "Restricted Code" },
    { "S-1-5-13",
      "Terminal Server Users" },
    { "S-1-5-14",
      "Remote Interactive Logon" },
    { "S-1-5-15",
      "This Organization" },
    { "S-1-5-17",
      "IIS User" },
    { "S-1-5-18",
      "Local System" },
    { "S-1-5-19",
      "NT Authority" },
    { "S-1-5-20",
      "NT Authority" },
/*
 * The S-1-5-21 prefix 21: SECURITY_NT_NON_UNIQUE, indicates a domain id will follow.
 * -domain- is actually 3 32-bit integers, followed by a -RID suffix.
 * Example: S-1-5-21-1-2-3-498; Enterprise RODC, with a sub-authority value of "1-2-3".
 */
#if 1
    { "S-1-5-21",
      "Enterprise Read-only Domain Controllers" },
#else
    { "S-1-5-21-X-Y-Z-498",
      "Enterprise Read-only Domain Controllers" },
    { "S-1-5-21-X-Y-Z-500",
      "Administrator" },
    { "S-1-5-21-X-Y-Z-501",
      "Guest" },
    { "S-1-5-21-X-Y-Z-502",
      "KRBTGT" },
    { "S-1-5-21-X-Y-Z-512",
      "Domain Admins" },
    { "S-1-5-21-X-Y-Z-513",
      "Domain Users" },
    { "S-1-5-21-X-Y-Z-514",
      "Domain Guests" },
    { "S-1-5-21-X-Y-Z-515",
      "Domain Computers" },
    { "S-1-5-21-X-Y-Z-516",
      "Domain Controllers" },
    { "S-1-5-21-X-Y-Z-517",
      "Cert Publishers" },
    { "S-1-5-21-X-Y-Z-518",
      "Schema Admins" },
    { "S-1-5-21-X-Y-Z-519",
      "Enterprise Admins" },
    { "S-1-5-21-X-Y-Z-520",
      "Group Policy Creator Owners" },
    { "S-1-5-21-X-Y-Z-521",
      "Read-only Domain Controllers" },
    { "S-1-5-21-X-Y-Z-522",
      "Cloneable Domain Controllers" },
    { "S-1-5-21-X-Y-Z-553",
      "RAS and IAS Servers" },
    { "S-1-5-21-X-Y-Z-571",
      "Allowed RODC Password Replication Group" },
    { "S-1-5-21-X-Y-Z-572",
      "Denied RODC Password Replication Group" },
#endif
    { "S-1-5-32-544",
      "Builtin Administrators" },
    { "S-1-5-32-545",
      "Builtin Users" },
    { "S-1-5-32-546",
      "Builtin Guests" },
    { "S-1-5-32-547",
      "Power Users" },
    { "S-1-5-32-548",
      "Account Operators" },
    { "S-1-5-32-549",
      "Server Operators" },
    { "S-1-5-32-550",
      "Print Operators" },
    { "S-1-5-32-551",
      "Backup Operators" },
    { "S-1-5-32-552",
      "Replicators" },
    { "S-1-5-64-10",
      "NTLM Authentication" },
    { "S-1-5-64-14",
      "SChannel Authentication" },
    { "S-1-5-64-21",
      "Digest Authentication" },
    { "S-1-5-80",
      "NT Service" },
    /*
     * NT Service account prefix
     */
    { "S-1-5-80-0",
/*      SID S-1-5-80-0 = NT SERVICES\\ALL SERVICES */
      "All Services" },
    { "S-1-5-83-0",
      "NT VIRTUAL MACHINE\\Virtual Machines" },
    { "S-1-16-0",
      "Untrusted Mandatory Level" },
    { "S-1-16-4096",
      "Low Mandatory Level" },
    { "S-1-16-8192",
      "Medium Mandatory Level" },
    { "S-1-16-8448",
      "Medium Plus Mandatory Level" },
    { "S-1-16-12288",
      "High Mandatory Level" },
    { "S-1-16-16384",
      "System Mandatory Level" },
    { "S-1-16-20480",
      "Protected Process Mandatory Level" },
    { "S-1-16-28672",
      "Secure Process Mandatory Level" },
    { "S-1-5-32-554",
      "BUILTIN\\Pre-Windows 2000 Compatible Access" },
    { "S-1-5-32-555",
      "BUILTIN\\Remote Desktop Users" },
    { "S-1-5-32-556",
      "BUILTIN\\Network Configuration Operators" },
    { "S-1-5-32-557",
      "BUILTIN\\Incoming Forest Trust Builders" },
    { "S-1-5-32-558",
      "BUILTIN\\Performance Monitor Users" },
    { "S-1-5-32-559",
      "BUILTIN\\Performance Log Users" },
    { "S-1-5-32-560",
      "BUILTIN\\Windows Authorization Access Group" },
    { "S-1-5-32-561",
      "BUILTIN\\Terminal Server License Servers" },
    { "S-1-5-32-562",
      "BUILTIN\\Distributed COM Users" },
    { "S-1-5-32-569",
      "BUILTIN\\Cryptographic Operators" },
    { "S-1-5-32-573",
      "BUILTIN\\Event Log Readers" },
    { "S-1-5-32-574",
      "BUILTIN\\Certificate Service DCOM Access" },
    { "S-1-5-32-575",
      "BUILTIN\\RDS Remote Access Servers" },
    { "S-1-5-32-576",
      "BUILTIN\\RDS Endpoint Servers" },
    { "S-1-5-32-577",
      "BUILTIN\\RDS Management Servers" },
    { "S-1-5-32-578",
      "BUILTIN\\Hyper-V Administrators" },
    { "S-1-5-32-579",
      "BUILTIN\\Access Control Assistance Operators" },
    { "S-1-5-32-580",
      "BUILTIN\\Remote Management Users" },
    { "S-1-18-1",
      "Authentication authority asserted identity" },
    { "S-1-18-2",
      "Service asserted identity" },
};
