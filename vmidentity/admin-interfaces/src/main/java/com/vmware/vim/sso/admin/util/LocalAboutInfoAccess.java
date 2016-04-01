/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.vim.sso.admin.util;

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.isEmpty;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * Convenient access to the build-time-defined component properties (a.k.a.
 * "About Info"). Currently includes:
 * <ul>
 * <li>Build number</li>
 * </ul>
 */
public final class LocalAboutInfoAccess {

   private static final String ABOUT_INFO_PROPERTIES = "/com/vmware/vim/sso/about-info.properties";
   private static final String ABOUT_BUILD_NUMBER = "build-number";
   private static final String ABOUT_API_REVISION = "api-revision";
   private static final String SSO_PRODUCT_VERSION_MAJOR = "sso-product-version-major";
   private static final String SSO_PRODUCT_VERSION_MINOR = "sso-product-version-minor";
   private static final String SSO_PRODUCT_VERSION_MAINT = "sso-product-version-maint";

   public static final class LocalAboutInfo {

      private final String buildNumber;
      private final String apiRevision;
      private final String ssoProductVersionMajor;
      private final String ssoProductVersionMinor;
      private final String ssoProductVersionMaint;

      private LocalAboutInfo(String buildNumber, String apiRevision,
            String ssoProductVersionMajor, String ssoProductVersionMinor, String ssoProductVersionMaint) {
         this.buildNumber = buildNumber;
         this.apiRevision = apiRevision;
         this.ssoProductVersionMajor = ssoProductVersionMajor;
         this.ssoProductVersionMinor = ssoProductVersionMinor;
         this.ssoProductVersionMaint = ssoProductVersionMaint;
      }

      public String getBuildNumber() {
         return buildNumber;
      }

      public String getApiRevision() {
         return apiRevision;
      }

      public String getSsoProductVersionMajor() {
         return ssoProductVersionMajor;
      }

      public String getSsoProductVersionMinor() {
         return ssoProductVersionMinor;
      }

      public String getSsoProductVersionMaint() {
         return ssoProductVersionMaint;
      }
      /**
       * @return The congregated product version: sso product version string,
       * followed by a dash, followed by the build number: {@code x.y.z-num}
       * indicates version {@code x.y.z} and build number {@code num}.
       * <p>Could be null when SSO product version information is not available.
       */
      public String getSsoProductInfo() {
         return ssoProductVersionMajor==null? null : String.format("%s.%s.%s-%s",
               ssoProductVersionMajor, ssoProductVersionMinor, ssoProductVersionMaint, buildNumber);
      }
   }

   public static final LocalAboutInfo LOCAL_ABOUT_INFO = loadLocalAboutInfo();

   /**
    * Helper: load the "about-info.properties" resource, throwing an
    * {@link Error} on failure.
    */
   private static LocalAboutInfo loadLocalAboutInfo() {
      InputStream aboutInfoStr = LocalAboutInfoAccess.class
         .getResourceAsStream(ABOUT_INFO_PROPERTIES);

      if (aboutInfoStr == null) {
         throw new ExceptionInInitializerError(String.format(
            "Missing required resources: %s (corrupt deployment?)",
            ABOUT_INFO_PROPERTIES));
      }

      Properties aboutInfoProps = new Properties();
      try {
         aboutInfoProps.load(aboutInfoStr);

         String buildNumber = aboutInfoProps.getProperty(ABOUT_BUILD_NUMBER);
         String apiRevision = aboutInfoProps.getProperty(ABOUT_API_REVISION);

         String ssoProductVersionMajor = aboutInfoProps.getProperty(SSO_PRODUCT_VERSION_MAJOR);
         String ssoProductVersionMinor = aboutInfoProps.getProperty(SSO_PRODUCT_VERSION_MINOR);
         String ssoProductVersionMaint = aboutInfoProps.getProperty(SSO_PRODUCT_VERSION_MAINT);

         if (isEmpty(buildNumber) || isEmpty(apiRevision)) {
            throw new ExceptionInInitializerError(
               "Missing required AboutInfo property");
         }

         return new LocalAboutInfo(buildNumber, apiRevision, ssoProductVersionMajor, ssoProductVersionMinor, ssoProductVersionMaint);

      } catch (IOException e) {
         throw new ExceptionInInitializerError(e);

      } catch (RuntimeException e) {
         throw new ExceptionInInitializerError(e);

      } finally {
         try {
            aboutInfoStr.close();
         } catch (IOException e) {
            throw new ExceptionInInitializerError(e);
         }
      }
   }
}
