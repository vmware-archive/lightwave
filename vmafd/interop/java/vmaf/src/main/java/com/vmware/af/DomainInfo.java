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

package com.vmware.af;

public class DomainInfo
{
    private final String _computerName;
    private final String _domainName;
    private final String _distinguishedName;
    private final String _netbiosName;

    public DomainInfo(final String computerName,
                      final String domainName,
                      final String distinguishedName,
                      final String netbiosName)
    {
       assert(computerName != null && !computerName.isEmpty());
       _computerName = computerName;
       _domainName = domainName;
       _distinguishedName = distinguishedName;
       _netbiosName = netbiosName;
    }

    public  String getDomainName()
    {
        return _domainName;
    }

    public String getComputerName()
    {
        return _computerName;
    }

    public String getDistinguishedName()
    {
        return _distinguishedName;
    }

    public String getNetbiosName()
    {
        return _netbiosName;
    }
}
