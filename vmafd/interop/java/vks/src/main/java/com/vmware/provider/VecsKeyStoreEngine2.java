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

package com.vmware.provider;

import java.io.InputStream;
import java.io.OutputStream;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.KeyStoreSpi;
import java.security.NoSuchAlgorithmException;

import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;

import com.vmware.identity.vecs.VMwareEndpointCertificateStore;
import com.vmware.identity.vecs.VecsEntry;
import com.vmware.identity.vecs.VecsEntryInfoLevel;
import com.vmware.identity.vecs.VecsEntryType;
import com.vmware.identity.vecs.VecsGenericException;
import com.vmware.identity.vecs.VecsStoreFactory;
import com.vmware.identity.vecs.VecsEntryEnumeration;

/**
 * VECS implementation of the KeyStoreSpi.
 */
final public class VecsKeyStoreEngine2 extends KeyStoreSpi {

   private String _storeName = null;
   private HashMap<String, VecsEntry> _vecsEntryLookup;

   public VecsKeyStoreEngine2()
   {
       _vecsEntryLookup = new HashMap<String, VecsEntry>();
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineAliases()
    */
   public Enumeration<String> engineAliases()
   {
      Vector<String> aliases = new Vector<String>();

      aliases.addAll(_vecsEntryLookup.keySet());

      return aliases.elements();
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineContainsAlias(java.lang.String)
    */
   public boolean engineContainsAlias(String alias)
   {
      return _vecsEntryLookup.containsKey(alias);
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineDeleteEntry(java.lang.String)
    */
   public void engineDeleteEntry(String alias) throws KeyStoreException
   {
      throw new KeyStoreException(
              String.format(
                      "Error: Modifying keystore is not permitted",
                      alias));
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetCertificate(java.lang.String)
    */
   public Certificate engineGetCertificate(String alias)
   {
      Certificate[] certChain = engineGetCertificateChain(alias);

      if (certChain == null || certChain.length == 0)
      {
         return null;
      }

      return certChain[0];
   }

   /*
    * (non-Javadoc)
    *
    * @see
    * java.security.KeyStoreSpi#engineGetCertificateAlias(java.security.cert
    * .Certificate)
    */
   public String engineGetCertificateAlias(Certificate cert)
   {
      if (!(cert instanceof X509Certificate))
      {
         return null;
      }

      try
      {
         Iterator<Map.Entry<String, VecsEntry>> iter =
                                     _vecsEntryLookup.entrySet().iterator();

         while (iter.hasNext())
         {
             Map.Entry<String, VecsEntry> pair = iter.next();

             String alias = pair.getKey();
             VecsEntry vecsEntry = pair.getValue();

             if (vecsEntry.certificateChain != null &&
                 cert.equals(vecsEntry.certificateChain[0]))
             {
                 return alias;
             }
         }
      }
      catch(VecsGenericException vge)
      {
         throw new VecsException(vge);
      }

      return null;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetCertificateChain(java.lang.String)
    */
   public Certificate[] engineGetCertificateChain(String alias)
   {
      VecsEntry entry = _vecsEntryLookup.get(alias);

      return entry == null ? null : entry.certificateChain;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetCreationDate(java.lang.String)
    */
   public Date engineGetCreationDate(String alias)
   {
      VecsEntry entry = _vecsEntryLookup.get(alias);

      return entry == null ? null : entry.date;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetKey(java.lang.String, char[])
    */
   public Key
   engineGetKey(
       String alias,
       char[] password
       ) throws UnrecoverableKeyException, NoSuchAlgorithmException
   {
      try
      {
         VMwareEndpointCertificateStore vecs = null;

         try
         {
             if (_storeName == null || _storeName.isEmpty())
             {
                 throw new IllegalStateException("Error: Found empty store name");
             }

             vecs = VecsStoreFactory.getVecsStoreFactoryViaIPC().getVecsStore(_storeName);

             vecs.openStore();

             return vecs.getKeyByAlias(alias, password);
         }
         finally
         {
             if (vecs != null)
             {
               vecs.closeStore();
             }
         }
      }
      catch(VecsGenericException vge)
      {
         throw new VecsException(vge);
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineIsCertificateEntry(java.lang.String)
    */
   public boolean engineIsCertificateEntry(String alias)
   {
      VecsEntry entry = _vecsEntryLookup.get(alias);

      boolean bRes = false;

      if (entry != null &&
          entry.entryType != null &&
          entry.entryType.equals(VecsEntryType.CERT_ENTRY_TYPE_TRUSTED_CERT))
      {
         bRes = true;
      }

      return bRes;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineIsKeyEntry(java.lang.String)
    */
   public boolean engineIsKeyEntry(String alias)
   {
      VecsEntry entry = _vecsEntryLookup.get(alias);

      boolean bRes = false;

      if (entry != null &&
          entry.entryType != null &&
          (entry.entryType.equals(VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY) ||
           entry.entryType.equals(VecsEntryType.CERT_ENTRY_TYPE_SECRET_KEY)))
      {
         bRes = true;
      }

      return bRes;
   }

   /*
    * (non-Javadoc)
    *
    * @see
    * java.security.KeyStoreSpi#engineLoad(java.security.KeyStore.LoadStoreParameter
    * )
    */
   @Override
   public void engineLoad(KeyStore.LoadStoreParameter param)
   {
      if (!(param instanceof VecsLoadStoreParameter))
      {
         throw new IllegalArgumentException(
               "'param' should be of type VecsLoadStoreParameter.");
      }

      VMwareEndpointCertificateStore vecs = null;
      VecsEntryEnumeration entryEnum = null;

      try
      {
         String storeName = ((VecsLoadStoreParameter) param).getStoreName();

         vecs = VecsStoreFactory.getVecsStoreFactoryViaIPC().getVecsStore(storeName);

         vecs.openStore();

         entryEnum = vecs.enumerateEntries(VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);

         while (entryEnum.hasMoreElements())
         {
             VecsEntry entry = entryEnum.nextElement();
             _vecsEntryLookup.put(entry.alias,  entry);
         }

         _storeName = storeName;
      }
      finally
      {
          if (entryEnum != null)
          {
             entryEnum.close();
          }
          if (vecs != null)
          {
              vecs.closeStore();
          }
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineLoad(java.io.InputStream, char[])
    */
   public void engineLoad(InputStream stream, char[] password)
   {
      throw new UnsupportedOperationException(
            "'load(InputStream, char[])' is not supported. Use 'load(LoadStoreParameter)'");
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineSetCertificateEntry(java.lang.String,
    * java.security.cert.Certificate)
    */
   public void engineSetCertificateEntry(String alias, Certificate cert)
         throws KeyStoreException
   {
       throw new KeyStoreException(
               String.format(
                       "Error: Modifying keystore is not permitted",
                       alias));
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineSetKeyEntry(java.lang.String, byte[],
    * java.security.cert.Certificate[])
    */
   public void engineSetKeyEntry(String alias, byte[] key, Certificate[] cert)
         throws KeyStoreException
   {
       throw new KeyStoreException(
               String.format(
                       "Error: Modifying keystore is not permitted",
                       alias));
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineSetKeyEntry(java.lang.String,
    * java.security.Key, char[], java.security.cert.Certificate[]) Ignoring Cert
    * and Password is by Design
    */
   public void
   engineSetKeyEntry(
       String alias,
       Key key,
       char[] password,
       Certificate[] chain
       ) throws KeyStoreException
   {
       throw new KeyStoreException(
               String.format(
                       "Error: Modifying keystore is not permitted",
                       alias));
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineSize()
    */
   public int engineSize()
   {
      return _vecsEntryLookup.size();
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineStore(java.security.KeyStore.
    * LoadStoreParameter)
    */
   @Override
   public void engineStore(KeyStore.LoadStoreParameter param)
   {
      throw new UnsupportedOperationException(
            "'store()' is not supported in VKS because it is instantly saved.");
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineStore(java.io.OutputStream, char[])
    */
   public void engineStore(OutputStream stream, char[] password)
   {
      throw new UnsupportedOperationException(
            "'store()' is not supported in VKS because it is instantly saved.");
   }
}
