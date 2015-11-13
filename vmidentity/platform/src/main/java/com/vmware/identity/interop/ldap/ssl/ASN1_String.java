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

package com.vmware.identity.interop.ldap.ssl;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/*
 *
ASN1_STRING, the real internal structure underlying the types ASN1_INTEGER, ASN1_BIT_STRING, ASN1_OCTET_STRING, ASN1_PRINTABLESTRING, ASN1_T61STRING, ASN1_IA5STRING, ASN1_UTCTIME, ASN1_GENERALIZEDTIME, ASN1_GENERALSTRING, ASN1_UNIVERSALSTRING, and ASN1_BMPSTRING

typedef struct asn1_string_st
        {
        int length;
        int type;
        unsigned char *data;
        } ASN1_STRING;
 */

public class ASN1_String extends Structure
{

   public int length;
   public int type;
   public Pointer data;

   public ASN1_String(Pointer p)
   {
      super();
      useMemory(p);
      read();
   }

   @Override
   protected List<String> getFieldOrder()
   {
      return Arrays.asList("length", "type", "data");
   }

}
