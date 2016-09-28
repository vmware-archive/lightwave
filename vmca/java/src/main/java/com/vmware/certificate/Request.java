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

package com.vmware.certificate;

import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;
import java.util.Date;


// import sun.security.pkcs.PKCS10;
// import sun.security.x509.X500Name;
// import sun.security.x509.X500Signer;

/**
 * @author Anu Engineer
 *
 */
public class Request {

   private static final int MIN_KEY_LENGTH = 1024;
   private static final String KEY_ALGORITHM_RSA = "RSA";
   /**
    * Ctor for the Request Object
    */
   public Request() {

      locality = "";
      state = "";
      organization = "";
      orgunit = "";
      dnsname = "";
      uri = "";
      email = "";
      ipaddress = "";
      keyusage = 0;

   }

   /**
    * Creates a Public / Private Key pair
    *
    * @param KeyLength
    *           -- Length of the Key , must be greater 1024
    * @return
    * @throws Exception
    */
   public KeyPair createKeyPair(Integer KeyLength) throws Exception {
      KeyPairGenerator keyGen = null;
      if (KeyLength < MIN_KEY_LENGTH) {
         throw new IllegalStateException(
          String.format("Minimum key length is %s bits", MIN_KEY_LENGTH));
      }

      try {
         keyGen = KeyPairGenerator.getInstance(KEY_ALGORITHM_RSA);
         keyGen.initialize(KeyLength, new SecureRandom());
         KeyPair keypair = keyGen.generateKeyPair();
         return keypair;
      } catch (NoSuchAlgorithmException e) {
         e.printStackTrace();
         throw e;
      }

   }

   /**
    * Returns the PKCS10 or Certificate Signing Request
    *
    * @param keys
    * @return
    * @throws Exception
    */
   public X509Certificate getCertificate(String ServerName, KeyPair keys,
                  Date tmNotBefore, Date tmNotAfter)  throws Exception {
      // PKCS10 pkcs = new PKCS10(keys.get((Public());
      // Signature signature = Signature.getInstance(SIG_ALGORITHM_SHA256);
      // signature.initSign(keys.getPrivate());
      // X500Name agentName =
      //       new X500Name(getName(), getOrgunit(), getOrganization(),
      //             getLocality(), getState(), getCountry());
      // X500Signer signer = new X500Signer(signature, agentName);
      // pkcs.encodeAndSign(signer);
      // return pkcs;
    return null;

   }


   /**
    * @return the name
    */
   public String getName() {
      return name;
   }

   /**
    * @param name
    *           the name to set
    */
   public void setName(String name) {
      this.name = name;
   }

   /**
    * @return the country
    */
   public String getCountry() {
      return country;
   }

   /**
    * @param country
    *           the country to set
    */
   public void setCountry(String country) {
      this.country = country;
   }

   /**
    * @return the locality
    */
   public String getLocality() {
      return locality;
   }

   /**
    * @param locality
    *           the locality to set
    */
   public void setLocality(String locality) {
      this.locality = locality;
   }

   /**
    * @return the state
    */
   public String getState() {
      return state;
   }

   /**
    * @param state
    *           the state to set
    */
   public void setState(String state) {
      this.state = state;
   }

   /**
    * @return the organization
    */
   public String getOrganization() {
      return organization;
   }

   /**
    * @param organization
    *           the organization to set
    */
   public void setOrganization(String organization) {
      this.organization = organization;
   }

   /**
    * @return the orgunit
    */
   public String getOrgunit() {
      return orgunit;
   }

   /**
    * @param orgunit
    *           the orgunit to set
    */
   public void setOrgunit(String orgunit) {
      this.orgunit = orgunit;
   }

   /**
    * @return the dnsname
    */
   public String getDnsname() {
      return dnsname;
   }

   /**
    * @param dnsname
    *           the dnsname to set
    */
   public void setDnsname(String dnsname) {
      this.dnsname = dnsname;
   }

   /**
    * @return the uri
    */
   public String getUri() {
      return uri;
   }

   /**
    * @param uri
    *           the uri to set
    */
   public void setUri(String uri) {
      this.uri = uri;
   }

   /**
    * @return the email
    */
   public String getEmail() {
      return email;
   }

   /**
    * @param email
    *           the email to set
    */
   public void setEmail(String email) {
      this.email = email;
   }

   /**
    * @return the ipaddress
    */
   public String getIpaddress() {
      return ipaddress;
   }

   /**
    * @param ipaddress
    *           the ipaddress to set
    */
   public void setIpaddress(String ipaddress) {
      this.ipaddress = ipaddress;
   }

   /**
    * @return the keyusage
    */
   public Integer getKeyusage() {
      return keyusage;
   }

   /**
    * @param keyusage
    *           the keyusage to set
    */
   public void setKeyusage(Integer keyusage) {
      this.keyusage = keyusage;
   }


   private String name;
   private String country;
   private String locality;
   private String state;
   private String organization;
   private String orgunit;
   private String dnsname;
   private String uri;
   private String email;
   private String ipaddress;
   private Integer keyusage;

}
