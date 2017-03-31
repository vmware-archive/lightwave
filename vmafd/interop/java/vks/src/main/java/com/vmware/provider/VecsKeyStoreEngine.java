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
import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.KeyStoreSpi;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.CRLException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Collections;
import java.util.Date;
import java.util.Enumeration;
import java.util.Vector;

import javax.crypto.spec.SecretKeySpec;

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
final public class VecsKeyStoreEngine extends KeyStoreSpi {

   private VMwareEndpointCertificateStore _vecs;
   private String _storeName = null;
   private final Object _monitor = new Object();

   public VecsKeyStoreEngine() {
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineAliases()
    */
   public Enumeration<String> engineAliases() {
      Enumeration<String> aliasEnum = Collections.emptyEnumeration();
      VecsEntryEnumeration entryEnum = null;
      try {
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               entryEnum = _vecs
                     .enumerateEntries(VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1);
            } finally {
               _vecs.closeStore();
            }
         }
        aliasEnum = getAliases(entryEnum);
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      } finally {
         if (entryEnum != null)
         {
            entryEnum.close();
         }
      }
      return aliasEnum;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineContainsAlias(java.lang.String)
    */
   public boolean engineContainsAlias(String alias) {
      VecsEntry entry = getEntryByAlias(alias,
            VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1);

      if (entry == null || entry.entryType == VecsEntryType.CERT_ENTRY_TYPE_CRL) {
         return false;
      } else {
         return true;
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineDeleteEntry(java.lang.String)
    */
   public void engineDeleteEntry(String alias) throws KeyStoreException {
      try {
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               _vecs.deleteEntryByAlias(alias);
            } finally {
               _vecs.closeStore();
            }
         }
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetCertificate(java.lang.String)
    */
   public Certificate engineGetCertificate(String alias) {
      Certificate[] certChain = engineGetCertificateChain(alias);
      if (certChain == null || certChain.length == 0) {
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
   public String engineGetCertificateAlias(Certificate cert) {
      if (!(cert instanceof X509Certificate)) {
         return null;
      }

      try {
         Enumeration<VecsEntry> entryEnum = null;
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               entryEnum = _vecs.enumerateEntries(VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);
            } finally {
               _vecs.closeStore();
            }
         }
         while (entryEnum.hasMoreElements()) {
            VecsEntry entry = entryEnum.nextElement();
            if (entry.certificateChain != null
                  && cert.equals(entry.certificateChain[0])) {
               return entry.alias;
            }
         }
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      }
      
      return null;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetCertificateChain(java.lang.String)
    */
   public Certificate[] engineGetCertificateChain(String alias) {
      VecsEntry entry = getEntryByAlias(alias,
            VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);
      return entry == null ? null : entry.certificateChain;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetCreationDate(java.lang.String)
    */
   public Date engineGetCreationDate(String alias) {
      VecsEntry entry = getEntryByAlias(alias,
            VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1);
      return entry == null ? null : entry.date;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineGetKey(java.lang.String, char[])
    */
   public Key engineGetKey(String alias, char[] password)
         throws UnrecoverableKeyException, NoSuchAlgorithmException {
      try{
         Key key = null;
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               key =  _vecs.getKeyByAlias(alias, password);
            } finally {
               _vecs.closeStore();
            }
         }
         return key;
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineIsCertificateEntry(java.lang.String)
    */
   public boolean engineIsCertificateEntry(String alias) {
      VecsEntry entry = getEntryByAlias(alias,
            VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1);

      boolean bRes = false;
      if (entry != null
            && entry.entryType != null
            && entry.entryType
                  .equals(VecsEntryType.CERT_ENTRY_TYPE_TRUSTED_CERT)) {
         bRes = true;
      }
      return bRes;
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineIsKeyEntry(java.lang.String)
    */
   public boolean engineIsKeyEntry(String alias) {
      VecsEntry entry = getEntryByAlias(alias,
            VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1);

      boolean bRes = false;
      if (entry != null
            && entry.entryType != null
            && (entry.entryType
                  .equals(VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY) || entry.entryType
                  .equals(VecsEntryType.CERT_ENTRY_TYPE_SECRET_KEY))) {
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
   public void engineLoad(KeyStore.LoadStoreParameter param) {
      if (!(param instanceof VecsLoadStoreParameter)) {
         throw new IllegalArgumentException(
               "'param' should be of type VecsLoadStoreParameter.");
      }
      _storeName = ((VecsLoadStoreParameter) param).getStoreName();
      synchronized(_monitor) {
         _vecs = VecsStoreFactory.getVecsStoreFactoryViaIPC().getVecsStore(_storeName);
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineLoad(java.io.InputStream, char[])
    */
   public void engineLoad(InputStream stream, char[] password) {
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
         throws KeyStoreException {
      try {
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               _vecs.addTrustedCertEntry(alias, (X509Certificate) cert);
            } finally {
               _vecs.closeStore();
            }
         }
      } catch (CertificateEncodingException e) {
         throw new KeyStoreException(
               "Getting encoded string of a certificate failed", e);
      } catch(VecsGenericException vge) {
            throw new VecsException(vge);
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.security.KeyStoreSpi#engineSetKeyEntry(java.lang.String, byte[],
    * java.security.cert.Certificate[])
    */
   public void engineSetKeyEntry(String alias, byte[] key, Certificate[] cert)
         throws KeyStoreException {
      throw new UnsupportedOperationException(
            "'engineSetKeyEntry(String, byte[], Certificate[])' is not supported. "
                  + "Please, use 'engineSetKeyEntry(String, Key, char[], Certificate[])'");
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineSetKeyEntry(java.lang.String,
    * java.security.Key, char[], java.security.cert.Certificate[]) Ignoring Cert
    * and Password is by Design
    */
   public void engineSetKeyEntry(String alias, Key key, char[] password,
         Certificate[] chain) throws KeyStoreException {
      if (key instanceof PrivateKey) {
         setPrivateKeyEntry(alias, (PrivateKey)key, password, chain);
      } else if (key instanceof SecretKeySpec) {
         if (chain != null && chain.length > 0) {
            throw new KeyStoreException(
                    "SecretKey should not have a certificate chain associated with it");
         }
         setSecretKeyEntry(alias, (SecretKeySpec)key, password);
      } else {
         throw new KeyStoreException("Cannot store keys which are not of class PrivateKey or SecretKeySpec");
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineSize()
    */
   public int engineSize() {
      try {
         int size;
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               size = _vecs.getEntryCount();
            } finally {
               _vecs.closeStore();
            }
         }
         return size;
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      }
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineStore(java.security.KeyStore.
    * LoadStoreParameter)
    */
   @Override
   public void engineStore(KeyStore.LoadStoreParameter param) {
      throw new UnsupportedOperationException(
            "'store()' is not supported in VKS because it is instantly saved. ");
   }

   /*
    * (non-Javadoc)
    *
    * @see java.security.KeyStoreSpi#engineStore(java.io.OutputStream, char[])
    */
   public void engineStore(OutputStream stream, char[] password) {
      throw new UnsupportedOperationException(
            "'store()' is not supported in VKS because it is instantly saved. ");
   }

   private VecsEntry getEntryByAlias(String alias, VecsEntryInfoLevel infoLevel) {
      VecsEntry entry = null;
      try {
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               entry = _vecs.getEntryByAlias(alias, infoLevel);
            } finally {
               _vecs.closeStore();
            }
         }
      } catch (CertificateException ce) {
         throw new IllegalStateException("Certificate failed to be generated.",
               ce);
      } catch (CRLException crle) {
          //since KeyStoreSpi should not deal with CRLs, we are ignoring this error.
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      }

      return entry;
   }

   private void setSecretKeyEntry(String alias, SecretKeySpec key, char[] password)
         throws KeyStoreException {
      synchronized(_monitor) {
         try {
            _vecs.openStore();
            _vecs.addSecretKeyEntry(alias, (SecretKeySpec) key, password);
         } finally {
            _vecs.closeStore();
         }
      }
   }

   private void setPrivateKeyEntry(String alias, PrivateKey key, char[] password, Certificate[] chain) throws KeyStoreException {
      if (chain == null || chain.length == 0) {
         throw new KeyStoreException(
               "PrivateKey should have a certificate chain associated with it");
      }

      try {
         synchronized(_monitor) {
            try {
               _vecs.openStore();
               _vecs.addPrivateKeyEntry(alias, (X509Certificate[]) chain,
                     (PrivateKey) key, password, false);
            } finally {
               _vecs.closeStore();
            }
         }
      } catch (CertificateEncodingException e) {
         throw new KeyStoreException(
               "Getting encoded string of a certificate failed", e);
      } catch (UnsupportedEncodingException e) {
         throw new KeyStoreException("Getting encoded string of a key failed",
               e);
      } catch(VecsGenericException vge) {
         throw new VecsException(vge);
      }
   }

   private Enumeration<String> getAliases(Enumeration<VecsEntry> entryEnum) {
		Vector<String> aliases = new Vector<String>();
		while (entryEnum.hasMoreElements()) {
			VecsEntry currEntry = entryEnum.nextElement();
			if (currEntry.entryType != VecsEntryType.CERT_ENTRY_TYPE_CRL) {
				aliases.add(currEntry.alias);
			}
		}
		return aliases.elements();
	}
}
