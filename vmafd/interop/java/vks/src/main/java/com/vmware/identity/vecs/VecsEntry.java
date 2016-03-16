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

package com.vmware.identity.vecs;

import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.Date;

/**
 * Represents a public information of VECS entry.
 */
public class VecsEntry {
   /**
    * Entry type.
    */
   public final VecsEntryType entryType;
   /**
    * Date an entry was created.
    */
   public final Date date;
   /**
    * Alias of an entry.
    */
   public final String alias;
   /**
    * Certificate chain associated with an entry.
    */
   public final X509Certificate[] certificateChain;

   public final X509CRL crl;

   VecsEntry(VecsEntryType entryTypeArg, Date dateArg, String aliasArg,
         X509Certificate[] certificateChainArg, X509CRL crlArg) {
      entryType = entryTypeArg;
      date = dateArg;
      alias = aliasArg;
      certificateChain = certificateChainArg;
      crl = crlArg;
   }
}
