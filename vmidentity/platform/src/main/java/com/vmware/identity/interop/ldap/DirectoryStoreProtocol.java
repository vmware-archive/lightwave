/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.net.URI;
import org.apache.http.conn.util.InetAddressUtils;

public enum DirectoryStoreProtocol
{
   LDAP("ldap"),
   LDAPS("ldaps");

   private String name;

   public static boolean isProtocolSupported(String scheme)
   {
      for (DirectoryStoreProtocol v :DirectoryStoreProtocol.values())
      {
         if (scheme.compareToIgnoreCase(v.getName()) == 0)
         {
            return true;
         }
      }
      return false;
   }

   public static DirectoryStoreProtocol getValue(String aName)
   {
      for (DirectoryStoreProtocol item : DirectoryStoreProtocol.values())
      {
         if (item.getName().compareToIgnoreCase(aName) == 0)
         {
            return item;
         }
      }
      return null;
   }

   DirectoryStoreProtocol(String aProtocolName)
   {
      this.name = aProtocolName.toLowerCase();
   }

   public String getName()
   {
      return this.name;
   }

   public URI getUri(String hostName, int port)
   {
      try {
         return new URI(String.format("%s://%s:%d", this.name, InetAddressUtils.isIPv6Address(hostName) ?
                 ("[" + hostName + "]") : hostName, port));
      }
      catch (Exception e)
      {
         return null;
      }
   }

}
