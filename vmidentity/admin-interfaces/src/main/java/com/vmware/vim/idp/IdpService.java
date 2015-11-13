/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp;

import java.net.URI;
import java.security.cert.X509Certificate;
import java.util.Collections;
import java.util.Set;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Service registration which is based on a particular IDP.
 * <p>
 * When used in keyed collection:<br>
 * Two IDP services are considered equals if their types match - {@link
 * #getType()}.
 */
public final class IdpService {

   private final URI _type;
   private final Set<Endpoint> _endpoints;

   /**
    * IDP service endpoint.
    * <p>
    * Contract when used in keyed collections:<br>
    * Two IDP service endpoints are considered equals if their URLs match -
    * {@link #getUrl()}.
    */
   public static final class Endpoint {
      private final X509Certificate _sslTrustAnchor;
      private final URI _url;

      /**
       * IDP endpoint Constructor
       *
       * @param url
       *           service URL; {@code not-null} value is required.
       * @param sslTrustAnchor
       *           SSL certificate used to verify the peer on; {@code not-null}
       *           value is required.
       */
      public Endpoint(URI url, X509Certificate sslTrustAnchor) {
         ValidateUtil.validateNotNull(url, "Url");
         ValidateUtil.validateNotNull(sslTrustAnchor, "SSL trust anchor");

         _url = url;
         _sslTrustAnchor = sslTrustAnchor;
      }

      /**
       * The trust anchor certificate used to validate the SSL certificate path
       * for this service. It should be base64 encoded DER format of X509. Can
       * be {@code null} If not {@code null} should be a valid, not expired
       * certificate.
       */
      public X509Certificate getSslTrustAnchor() {
         return _sslTrustAnchor;
      }

      /**
       * The URL of this endpoint. Cannot be {@code null}, empty or exceed 2048
       * chars. Should be valid URI.
       */
      public URI getUrl() {
         return _url;
      }

      @Override
      public boolean equals(Object other) {
         return other != null && other instanceof IdpService
            && _url.equals(((IdpService) other)._type);
      }

      @Override
      public int hashCode() {
         return _url.hashCode();
      }

      @Override
      public String toString() {
         return String.format("Endpoint: (uri='%s', SSLCertIssuer='%s')", _url,
            _sslTrustAnchor.getIssuerDN());
      }
   }

   /**
    * IDP service constructor
    *
    * @param type
    *           service type; {@code not-null} value is required
    * @param endpoints
    *           service endpoints; {@code not-empty} collection is required
    */
   public IdpService(URI type, Set<Endpoint> endpoints) {
      ValidateUtil.validateNotNull(type, "ServiceType");
      ValidateUtil.validateNotEmpty(endpoints, "Endpoints");

      _type = type;
      _endpoints = Collections.unmodifiableSet(endpoints);
   }

   /**
    * @return service type
    */
   public URI getType() {
      return _type;
   }

   /**
    * @return unmodifiable set of all endpoints for this service
    */
   public Set<Endpoint> getEndpoints() {
      return _endpoints;
   }

   /**
    * Returns @ code true} when compared with another IdpService instance having
    * equals service type
    * @see #getType()
    */
   @Override
   public boolean equals(Object obj) {
      if (obj == null || !(obj instanceof IdpService)) {
         return false;
      }
      IdpService _other = (IdpService) obj;
      return _type.equals(_other._type);
   }

   /**
    * Calculated on service type field.
    */
   @Override
   public int hashCode() {
      return _type.hashCode();
   }

   @Override
   public String toString() {
      return String.format("Service (type: %s, endpoints: %d)", _type,
         _endpoints.size());
   }

}
