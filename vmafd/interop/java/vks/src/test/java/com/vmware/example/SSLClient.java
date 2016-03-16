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

package com.vmware.example;
 
import java.net.InetSocketAddress;
import java.net.URL;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.io.*;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import com.vmware.provider.VecsLoadStoreParameter;
 
public class SSLClient{
   private static final int CONNECTION_TIMEOUT_MS = 60 * 1000; // 1 minute
   public static void main(String[] args) throws Exception {
      new SSLClient().testIt();
   }
 
   private void testIt() throws Exception{
      KeyStore ks = KeyStore.getInstance("VKS");  
      ks.load(new VecsLoadStoreParameter("store-name")); 
        
      URL address= new URL("https://localhost");

      SSLSocket socket = null;

     
      TrustManager trm = new CustomizedTrustManager(ks);
      SSLContext sc = SSLContext.getInstance("SSL");
      sc.init(null, new TrustManager[] { trm }, null);
      SSLSocketFactory factory = sc.getSocketFactory();
      int port = address.getPort();
      port = port == -1 ? 443 : port;
      socket = (SSLSocket) factory.createSocket();
      InetSocketAddress inetAddress = new InetSocketAddress(
            address.getHost(), port);

        
        socket.connect(inetAddress, CONNECTION_TIMEOUT_MS);
        socket.setSoTimeout(CONNECTION_TIMEOUT_MS);
        socket.startHandshake();
        InputStream istr = socket.getInputStream();
        
        byte[] buffer = new byte[1000];
        int rBytes = istr.read(buffer);
        String str = new String(buffer);
        System.out.println("read "+ str);
        System.out.println("read bytes "+ rBytes);
        
        istr.close();
        socket.close();
   }
  
  private static final class CustomizedTrustManager implements X509TrustManager {
     private final X509TrustManager defaultPkixTrustMgr;

     public CustomizedTrustManager(KeyStore trustStore) throws NoSuchAlgorithmException, KeyStoreException {

        TrustManagerFactory mgrFactory = TrustManagerFactory.getInstance("PKIX");
        mgrFactory.init(trustStore);
        TrustManager tms [] = mgrFactory.getTrustManagers(); 
       
        X509TrustManager tm = null;
      for (int i = 0; i < tms.length; i++) {  
          if (tms[i] instanceof X509TrustManager) {  
              tm = (X509TrustManager) tms[i];  
              break;  
          }  
      }
      defaultPkixTrustMgr = tm;
     }

     @Override
     public void checkClientTrusted(X509Certificate[] chain, String authType)
        throws CertificateException {
        return;
     }

     @Override
     public void checkServerTrusted(X509Certificate[] chain, String authType)
        throws CertificateException {
           defaultPkixTrustMgr.checkServerTrusted(chain, authType);
     }

     @Override
     public X509Certificate[] getAcceptedIssuers() {
        return new X509Certificate[0];
     }
  } 
}
